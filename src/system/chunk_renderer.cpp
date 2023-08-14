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

private:
  struct BlockTexture
  {
    std::uint32_t texture_indices[6];
  };

public:
  ChunkRendererSystem(const WorldConfig& world_config)
  {
    // 1: ShaderProgram
    m_shader_program = std::make_unique<graphics::ShaderProgram>("assets/chunk.vert", "assets/chunk.frag");

    // 2: TextureArray
    std::vector<std::string> texture_filenames;
    for(const BlockConfig& block_config : world_config.blocks)
      for(const std::string& texture_filename : block_config.textures)
        texture_filenames.push_back(texture_filename);

    std::sort(texture_filenames.begin(), texture_filenames.end());
    texture_filenames.erase(std::unique(texture_filenames.begin(), texture_filenames.end()), texture_filenames.end());

    m_blocks_texture_array = std::make_unique<graphics::TextureArray>(texture_filenames);

    // 3: Build lookup table from block id to texture indices
    std::unordered_map<std::string, std::uint32_t> texture_indices_map;
    texture_indices_map.reserve(texture_filenames.size());
    for(size_t i=0; i<texture_filenames.size(); ++i)
      texture_indices_map.emplace(texture_filenames[i], i);

    for(const BlockConfig& block_config : world_config.blocks)
    {
      BlockTexture block_texture;
      for(size_t i=0; i<6; ++i)
        block_texture.texture_indices[i] = texture_indices_map.at(block_config.textures[i]);
      m_block_textures.push_back(block_texture);
    }
  }

private:
  graphics::Mesh generate_chunk_mesh(const WorldData& world, glm::ivec2 chunk_index, const ChunkData& chunk) const
  {
    struct Vertex
    {
      glm::vec3 position;
      glm::vec2 texture_coords;
      uint32_t  texture_index;
      float     light_level;
      float     destroy_level;
    };

    std::vector<uint32_t> indices;
    std::vector<Vertex>   vertices;
    for(int lz=0; lz<CHUNK_HEIGHT; ++lz)
      for(int ly=0; ly<CHUNK_WIDTH; ++ly)
        for(int lx=0; lx<CHUNK_WIDTH; ++lx)
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

            const BlockTexture& block_texture = m_block_textures.at(block->id);
            uint32_t texture_index = block_texture.texture_indices[i];
            float    light_level   = (neighbour_block ? neighbour_block->light_level : 15) / 16.0f;
            float    destroy_level = block->destroy_level / 16.0f;

            vertices.push_back(Vertex{ .position = center + ( - 0.5f * glm::vec3(right) - 0.5f * glm::vec3(up)), .texture_coords = {0.0f, 0.0f}, .texture_index = texture_index, .light_level = light_level, .destroy_level = destroy_level, });
            vertices.push_back(Vertex{ .position = center + ( + 0.5f * glm::vec3(right) - 0.5f * glm::vec3(up)), .texture_coords = {1.0f, 0.0f}, .texture_index = texture_index, .light_level = light_level, .destroy_level = destroy_level, });
            vertices.push_back(Vertex{ .position = center + ( - 0.5f * glm::vec3(right) + 0.5f * glm::vec3(up)), .texture_coords = {0.0f, 1.0f}, .texture_index = texture_index, .light_level = light_level, .destroy_level = destroy_level, });
            vertices.push_back(Vertex{ .position = center + ( + 0.5f * glm::vec3(right) + 0.5f * glm::vec3(up)), .texture_coords = {1.0f, 1.0f}, .texture_index = texture_index, .light_level = light_level, .destroy_level = destroy_level, });
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
          { .type = graphics::AttributeType::FLOAT1,        .offset = offsetof(Vertex, destroy_level),  },
        },
    };

    return graphics::Mesh(
      std::move(layout),
      graphics::as_bytes(indices),
      graphics::as_bytes(vertices)
    );
  }

  void on_update(Application& application, const WorldConfig& world_config, WorldData& world_data, float dt) override
  {
    for(auto& [chunk_index, chunk] : world_data.dimension.chunks)
    {
      double time = glfwGetTime();
      if(chunk.mesh_invalidated_major || (chunk.mesh_invalidated_minor && (time - chunk.last_remash_time) >= REMASH_THROTTLE))
      {
        chunk.mesh_invalidated_major = false;
        chunk.mesh_invalidated_minor = false;
        chunk.last_remash_time = time;
        m_chunk_meshes.insert_or_assign(chunk_index, generate_chunk_mesh(world_data, chunk_index, chunk));
      }
    }
  }

  void on_render(Application& application, const WorldConfig& config, const WorldData& world) override
  {
    glUseProgram(m_shader_program->id());

    glm::mat4 view       = world.camera.view();
    glm::mat4 projection = world.camera.projection();
    glm::mat4 model = glm::mat4(1.0f);

    glm::mat4 MVP = projection * view * model;
    glm::mat4 MV  =              view * model;

    glUniformMatrix4fv(glGetUniformLocation(m_shader_program->id(), "MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
    glUniformMatrix4fv(glGetUniformLocation(m_shader_program->id(), "MV"),  1, GL_FALSE, glm::value_ptr(MV));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_blocks_texture_array->id());
    glUniform1i(glGetUniformLocation(m_shader_program->id(), "blocksTextureArray"), 0);
    for(const auto& [chunk_index, mesh] : m_chunk_meshes)
        mesh.draw_triangles();
  }


private:
  std::unique_ptr<graphics::ShaderProgram> m_shader_program;
  std::unique_ptr<graphics::TextureArray>  m_blocks_texture_array;
  std::vector<BlockTexture>                m_block_textures;

  std::unordered_map<glm::ivec2, graphics::Mesh> m_chunk_meshes;
};

std::unique_ptr<System> create_chunk_renderer_system(const WorldConfig& world_config)
{
  return std::make_unique<ChunkRendererSystem>(world_config);
}

