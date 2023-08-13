#include <system/chunk_renderer.hpp>

#include <world.hpp>
#include <coordinates.hpp>
#include <directions.hpp>

#include <graphics/shader_program.hpp>
#include <graphics/mesh.hpp>

#include <GLFW/glfw3.h>

#include <glm/gtc/type_ptr.hpp>

class ChunkRendererSystem : public System
{
private:
  static constexpr double REMASH_THROTTLE = 5.0f;

public:
  ChunkRendererSystem() :
    m_shader_program("assets/chunk.vert", "assets/chunk.frag"),
    m_blocks_texture_array({
      "assets/stone.png",
      "assets/grass_bottom.png",
      "assets/grass_side.png",
      "assets/grass_top.png",
    }),
    m_block_datas{
      { .texture_indices = {0, 0, 0, 0, 0, 0} },
      { .texture_indices = {2, 2, 2, 2, 1, 3} },
    }
  {}

private:
  graphics::Mesh generate_chunk_mesh(const World& world, glm::ivec2 chunk_index, const Chunk& chunk) const
  {
    struct Vertex
    {
      glm::vec3 position;
      glm::vec2 texture_coords;
      uint32_t  texture_index;
      float     light_level;
    };

    std::vector<uint32_t> indices;
    std::vector<Vertex>   vertices;
    for(int lz=0; lz<Chunk::HEIGHT; ++lz)
      for(int ly=0; ly<Chunk::WIDTH; ++ly)
        for(int lx=0; lx<Chunk::WIDTH; ++lx)
        {
          glm::ivec3   position = coordinates::local_to_global(glm::ivec3(lx, ly, lz), chunk_index);
          const Block* block    = world.get_block(position);
          if(block->id == Block::ID_NONE)
            continue;

          for(int i=0; i<std::size(DIRECTIONS); ++i)
          {
            glm::ivec3 direction = DIRECTIONS[i];

            glm::ivec3   neighbour_position = position + direction;
            const Block* neighbour_block    = world.get_block(neighbour_position);
            if(neighbour_block && neighbour_block->id != Block::ID_NONE)
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

            const BlockData& block_data = m_block_datas.at(block->id);
            uint32_t texture_index = block_data.texture_indices[i];
            float    light_level   = (neighbour_block ? neighbour_block->light_level : 15) / 16.0f;

            vertices.push_back(Vertex{ .position = center + ( - 0.5f * glm::vec3(right) - 0.5f * glm::vec3(up)), .texture_coords = {0.0f, 0.0f}, .texture_index = texture_index, .light_level = light_level, });
            vertices.push_back(Vertex{ .position = center + ( + 0.5f * glm::vec3(right) - 0.5f * glm::vec3(up)), .texture_coords = {1.0f, 0.0f}, .texture_index = texture_index, .light_level = light_level, });
            vertices.push_back(Vertex{ .position = center + ( - 0.5f * glm::vec3(right) + 0.5f * glm::vec3(up)), .texture_coords = {0.0f, 1.0f}, .texture_index = texture_index, .light_level = light_level, });
            vertices.push_back(Vertex{ .position = center + ( + 0.5f * glm::vec3(right) + 0.5f * glm::vec3(up)), .texture_coords = {1.0f, 1.0f}, .texture_index = texture_index, .light_level = light_level, });
            // NOTE: Brackets added so that it is possible for the compiler to do constant folding if loop is unrolled, not that it would actually do it.
          }
        }

    graphics::MeshLayout layout{
      .index_type = graphics::IndexType::UNSIGNED_INT,
        .stride = sizeof(Vertex),
        .attributes = {
          { .type = graphics::AttributeType::FLOAT3,        .offset = offsetof(Vertex, position),       },
          { .type = graphics::AttributeType::FLOAT2,        .offset = offsetof(Vertex, texture_coords), },
          { .type = graphics::AttributeType::UNSIGNED_INT1, .offset = offsetof(Vertex, texture_index),  },
          { .type = graphics::AttributeType::FLOAT1,        .offset = offsetof(Vertex, light_level),    },
        },
    };

    return graphics::Mesh(
      std::move(layout),
      graphics::as_bytes(indices),
      graphics::as_bytes(vertices)
    );
  }

  void on_update(Application& application, World& world, float dt) override
  {
    for(auto& [chunk_index, chunk] : world.dimension.chunks)
    {
      double time = glfwGetTime();
      if(chunk.mesh_invalidated_major || (chunk.mesh_invalidated_minor && (time - chunk.last_remash_time) >= REMASH_THROTTLE))
      {
        chunk.mesh_invalidated_major = false;
        chunk.mesh_invalidated_minor = false;
        chunk.last_remash_time = time;
        m_chunk_meshes.insert_or_assign(chunk_index, generate_chunk_mesh(world, chunk_index, chunk));
      }
    }
  }

  void on_render(Application& application, const World& world) override
  {
    glUseProgram(m_shader_program.id());

    glm::mat4 view       = world.camera.view();
    glm::mat4 projection = world.camera.projection();
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 MVP   = projection * view * model;
    glUniformMatrix4fv(glGetUniformLocation(m_shader_program.id(), "MVP"), 1, GL_FALSE, glm::value_ptr(MVP));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_blocks_texture_array.id());
    glUniform1i(glGetUniformLocation(m_shader_program.id(), "blocksTextureArray"), 0);
    for(const auto& [chunk_index, mesh] : m_chunk_meshes)
        mesh.draw_triangles();
  }


private:
  graphics::ShaderProgram m_shader_program;
  graphics::TextureArray  m_blocks_texture_array;
  std::vector<BlockData>  m_block_datas;

  std::unordered_map<glm::ivec2, graphics::Mesh> m_chunk_meshes;
};

std::unique_ptr<System> create_chunk_renderer_system()
{
  return std::make_unique<ChunkRendererSystem>();
}

