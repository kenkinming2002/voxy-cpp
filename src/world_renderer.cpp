#include <world_renderer.hpp>

#include <coordinates.hpp>
#include <directions.hpp>

#include <GLFW/glfw3.h>

#include <glm/gtc/type_ptr.hpp>

WorldRenderer::WorldRenderer(const WorldConfig& config)
{
  // 1: Build BlockRenderInfo[]
  std::unordered_set<std::string> block_texture_filenames_set;
  for(const BlockConfig& block_config : config.blocks)
    for(const std::string& block_texture_filename : block_config.textures)
      block_texture_filenames_set.insert(block_texture_filename);

  std::vector<std::string>                block_texture_filenames;
  std::unordered_map<std::string, size_t> block_texture_indices;

  block_texture_filenames.reserve(block_texture_filenames_set.size());
  block_texture_indices  .reserve(block_texture_filenames_set.size());

  size_t i = 0;
  for(const std::string& block_texture_filename : block_texture_filenames_set)
  {
    block_texture_filenames.push_back(block_texture_filename);
    block_texture_indices  .emplace(block_texture_filename, i++);
  }

  m_block_render_infos.reserve(config.blocks.size());
  for(const BlockConfig& block_config : config.blocks)
  {
    BlockRenderInfo block_render_info = {};
    for(unsigned i=0; i<6; ++i)
      block_render_info.texture_indices[i] = block_texture_indices.at(block_config.textures[i]);
    m_block_render_infos.push_back(block_render_info);
  }

  m_chunk_shader_program = std::make_unique<graphics::ShaderProgram>("assets/chunk.vert", "assets/chunk.frag");
  m_chunk_texture_array  = std::make_unique<graphics::TextureArray>(block_texture_filenames);

  // 2: Entity
  for(const EntityConfig& entity_config : config.entities)
    m_entity_render_infos.push_back(EntityRenderInfo{
      .mesh    = std::make_unique<graphics::Mesh>(entity_config.model),
      .texture = std::make_unique<graphics::Texture>(entity_config.texture),
    });

  m_entity_shader_program = std::make_unique<graphics::ShaderProgram>("assets/entity.vert", "assets/entity.frag");

}

void WorldRenderer::render(const Camera& camera, const World& world, bool third_person)
{
  render_chunks(camera, world);
  render_entites(camera, world, third_person);
}

void WorldRenderer::render_chunks(const Camera& camera, const World& world)
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

  graphics::MeshLayout mesh_layout{
    .index_type = graphics::IndexType::UNSIGNED_INT,
      .stride = sizeof(Vertex),
      .attributes = {
        { .type = graphics::AttributeType::FLOAT3,        .offset = offsetof(Vertex, position),       },
        { .type = graphics::AttributeType::FLOAT2,        .offset = offsetof(Vertex, texture_coords), },
        { .type = graphics::AttributeType::UNSIGNED_INT1, .offset = offsetof(Vertex, texture_index),  },
        { .type = graphics::AttributeType::FLOAT1,        .offset = offsetof(Vertex, light_ratio),    },
        { .type = graphics::AttributeType::FLOAT1,        .offset = offsetof(Vertex, destroy_ratio),  },
      },
  };

  std::vector<uint32_t> indices;
  std::vector<Vertex>   vertices;

  for(auto& [chunk_index, chunk] : world.dimension.chunks)
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

              const BlockRenderInfo& block_render_info = m_block_render_infos.at(block->id);
              uint32_t texture_index = block_render_info.texture_indices[i];
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

      m_chunk_meshes.insert_or_assign(chunk_index, graphics::Mesh(mesh_layout, graphics::as_bytes(indices), graphics::as_bytes(vertices)));
    }

  // 2: Rendering
  glUseProgram(m_chunk_shader_program->id());

  glm::mat4 view       = camera.view();
  glm::mat4 projection = camera.projection();
  glm::mat4 model      = glm::mat4(1.0f);

  glm::mat4 MVP = projection * view * model;
  glm::mat4 MV  =              view * model;

  glUniformMatrix4fv(glGetUniformLocation(m_chunk_shader_program->id(), "MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
  glUniformMatrix4fv(glGetUniformLocation(m_chunk_shader_program->id(), "MV"),  1, GL_FALSE, glm::value_ptr(MV));

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D_ARRAY, m_chunk_texture_array->id());
  glUniform1i(glGetUniformLocation(m_chunk_shader_program->id(), "blocksTextureArray"), 0);
  for(const auto& [chunk_index, mesh] : m_chunk_meshes)
    mesh.draw_triangles();
}

void WorldRenderer::render_entites(const Camera& camera, const World& world, bool third_person)
{
  glUseProgram(m_entity_shader_program->id());
  for(size_t i=0; i<world.dimension.entities.size(); ++i)
  {
    if(!third_person && i == world.player.entity_id)
      continue;

    const Entity&           entity             = world.dimension.entities[i];
    const EntityRenderInfo& entity_render_info = m_entity_render_infos.at(entity.id);

    glm::mat4 view       = camera.view();
    glm::mat4 projection = camera.projection();
    glm::mat4 model      = entity.transform.as_matrix_no_pitch_roll();

    glm::mat4 MVP = projection * view * model;
    glm::mat4 MV  =              view * model;

    glUniformMatrix4fv(glGetUniformLocation(m_entity_shader_program->id(), "MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
    glUniformMatrix4fv(glGetUniformLocation(m_entity_shader_program->id(), "MV"),  1, GL_FALSE, glm::value_ptr(MV));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, entity_render_info.texture->id());
    glUniform1i(glGetUniformLocation(m_entity_shader_program->id(), "ourTexture"), 0);
    entity_render_info.mesh->draw_triangles();
  }
}

