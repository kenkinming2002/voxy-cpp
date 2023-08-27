#include <world_renderer.hpp>

#include <coordinates.hpp>
#include <directions.hpp>

#include <GLFW/glfw3.h>

WorldRenderer::WorldRenderer(ResourcePack resource_pack) : m_resource_pack(std::move(resource_pack))
{
  m_chunk_shader_program = std::make_unique<graphics::ShaderProgram>("assets/chunk.vert", "assets/chunk.frag");
  m_entity_shader_program = std::make_unique<graphics::ShaderProgram>("assets/entity.vert", "assets/entity.frag");
}

void WorldRenderer::render(const graphics::Camera& camera, const World& world, bool third_person, graphics::WireframeRenderer& wireframe_renderer)
{
  render_chunks(camera, world);
  render_entites(camera, world, third_person, wireframe_renderer);
}

void WorldRenderer::render_chunks(const graphics::Camera& camera, const World& world)
{
  // 1: Mesh building
  struct Vertex
  {
    glm::vec3 position;
    glm::vec2 texture_coords;
    uint32_t  texture_index;
    float     light_ratio;
    float     destroy_ratio;
  };

  std::vector<uint32_t> indices;
  std::vector<Vertex>   vertices;

  for(auto& [chunk_index, chunk] : world.chunks)
    if(chunk.mesh_invalidated)
    {
      chunk.mesh_invalidated = false;

      indices.clear();
      vertices.clear();

      for(int lz=0; lz<CHUNK_HEIGHT; ++lz)
        for(int ly=0; ly<CHUNK_WIDTH; ++ly)
          for(int lx=0; lx<CHUNK_WIDTH; ++lx)
          {
            glm::ivec3   position = coordinates::local_to_global(glm::ivec3(lx, ly, lz), chunk_index);
            const Block* block    = get_block(world, position);
            if(block->id == BLOCK_ID_NONE)
              continue;

            for(int i=0; i<std::size(DIRECTIONS); ++i)
            {
              glm::ivec3 direction = DIRECTIONS[i];

              glm::ivec3   neighbour_position = position + direction;
              const Block* neighbour_block    = get_block(world, neighbour_position);
              if(neighbour_block && neighbour_block->id != BLOCK_ID_NONE)
                continue;

              uint32_t index_base = vertices.size();
              indices.push_back(index_base + 0);
              indices.push_back(index_base + 1);
              indices.push_back(index_base + 2);
              indices.push_back(index_base + 2);
              indices.push_back(index_base + 1);
              indices.push_back(index_base + 3);

              glm::ivec3 out   = direction;
              glm::ivec3 up    = direction.z == 0.0 ? glm::ivec3(0, 0, 1) : glm::ivec3(1, 0, 0);
              glm::ivec3 right = glm::cross(glm::vec3(up), glm::vec3(out));
              glm::vec3 center = glm::vec3(position) + glm::vec3(0.5f, 0.5f, 0.5f) + 0.5f * glm::vec3(out);

              const BlockResource& block_resource = m_resource_pack.blocks.at(block->id);
              uint32_t texture_index = block_resource.texture_indices[i];
              uint32_t light_level   = neighbour_block ? neighbour_block->light_level : 15;
              uint32_t destroy_level = block->destroy_level;

              float light_ratio   = light_level   / 16.0f;
              float destroy_ratio = destroy_level / 16.0f;

              vertices.push_back(Vertex{ .position = center + ( - 0.5f * glm::vec3(right) - 0.5f * glm::vec3(up)), .texture_coords = {0.0f, 0.0f}, .texture_index = texture_index, .light_ratio = light_ratio, .destroy_ratio = destroy_ratio, });
              vertices.push_back(Vertex{ .position = center + ( + 0.5f * glm::vec3(right) - 0.5f * glm::vec3(up)), .texture_coords = {1.0f, 0.0f}, .texture_index = texture_index, .light_ratio = light_ratio, .destroy_ratio = destroy_ratio, });
              vertices.push_back(Vertex{ .position = center + ( - 0.5f * glm::vec3(right) + 0.5f * glm::vec3(up)), .texture_coords = {0.0f, 1.0f}, .texture_index = texture_index, .light_ratio = light_ratio, .destroy_ratio = destroy_ratio, });
              vertices.push_back(Vertex{ .position = center + ( + 0.5f * glm::vec3(right) + 0.5f * glm::vec3(up)), .texture_coords = {1.0f, 1.0f}, .texture_index = texture_index, .light_ratio = light_ratio, .destroy_ratio = destroy_ratio, });
              // NOTE: Brackets added so that it is possible for the compiler to do constant folding if loop is unrolled, not that it would actually do it.
            }
          }

      auto it = m_chunk_meshes.find(chunk_index);
      if(it == m_chunk_meshes.end())
      {
        const graphics::Attribute attributes[] = {
          { .type = graphics::AttributeType::FLOAT3,        .offset = offsetof(Vertex, position),       },
          { .type = graphics::AttributeType::FLOAT2,        .offset = offsetof(Vertex, texture_coords), },
          { .type = graphics::AttributeType::UNSIGNED_INT1, .offset = offsetof(Vertex, texture_index),  },
          { .type = graphics::AttributeType::FLOAT1,        .offset = offsetof(Vertex, light_ratio),    },
          { .type = graphics::AttributeType::FLOAT1,        .offset = offsetof(Vertex, destroy_ratio),  },
        };

        std::unique_ptr<graphics::Mesh> chunk_mesh = std::make_unique<graphics::Mesh>(
          graphics::IndexType::UNSIGNED_INT,
          graphics::PrimitiveType::TRIANGLES,
          sizeof(Vertex),
          attributes);

        bool success;
        std::tie(it, success) = m_chunk_meshes.emplace(chunk_index, std::move(chunk_mesh));
        assert(success);
      }
      it->second->write(std::as_bytes(std::span(indices)), std::as_bytes(std::span(vertices)), graphics::Usage::DYNAMIC);
    }

  // 2: Rendering
  m_chunk_shader_program->use();

  glm::mat4 view       = camera.view();
  glm::mat4 projection = camera.projection();
  glm::mat4 model      = glm::mat4(1.0f);

  m_chunk_shader_program->set_uniform("MVP", projection * view * model);
  m_chunk_shader_program->set_uniform("MV",               view * model);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D_ARRAY, m_resource_pack.blocks_texture_array->id());
  m_chunk_shader_program->set_uniform( "blocksTextureArray", 0);

  for(const auto& [chunk_index, mesh] : m_chunk_meshes)
    mesh->draw();
}

void WorldRenderer::render_entites(const graphics::Camera& camera, const World& world, bool third_person, graphics::WireframeRenderer& wireframe_renderer)
{
  const Player& player = world.players.front();

  m_entity_shader_program->use();
  for(size_t i=0; i<world.entities.size(); ++i)
  {
    if(!third_person && i == player.entity_id)
      continue;

    const Entity&         entity          = world.entities[i];
    const EntityResource& entity_resource = m_resource_pack.entities.at(entity.id);

    glm::mat4 view       = camera.view();
    glm::mat4 projection = camera.projection();
    glm::mat4 model      = entity.transform.as_matrix_no_pitch_roll();

    m_entity_shader_program->set_uniform("MVP", projection * view * model);
    m_entity_shader_program->set_uniform("MV",               view * model);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, entity_resource.texture->id());
    m_entity_shader_program->set_uniform("ourTexture", 0);
    entity_resource.mesh->draw();
  }

  for(size_t i=0; i<world.entities.size(); ++i)
  {
    if(!third_person && i == player.entity_id)
      continue;

    const Entity& entity = world.entities[i];

    AABB entity_aabb = entity_get_aabb(entity);
    wireframe_renderer.render_cube(camera, entity_aabb.position, entity_aabb.dimension, glm::vec3(0.6f, 0.6f, 0.6f), 5.0f);
  }
}

