#include <system/chunk_mesher.hpp>

#include <world.hpp>

#include <SDL.h>

#include <spdlog/spdlog.h>

#include <glm/glm.hpp>

static constexpr float REMASH_THROTTLE = 5.0f;

class ChunkMesherSystem : public System
{
private:
  void on_update(World& world, float dt) override
  {
    for(auto& [chunk_index, chunk] : world.dimension.chunks)
      if(chunk.data)
      {
        uint32_t tick = SDL_GetTicks();
        if(!chunk.mesh || chunk.mesh_invalidated_major || (chunk.mesh_invalidated_minor && (tick - chunk.last_remash_tick) / 1000.0f >= REMASH_THROTTLE))
        {
          chunk.mesh_invalidated_major = false;
          chunk.mesh_invalidated_minor = false;
          chunk.last_remash_tick = tick;
          remesh_chunk(world, chunk_index, chunk);
        }
      }
  }

  void remesh_chunk(World& world, glm::ivec2 chunk_index, Chunk& chunk)
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
          glm::ivec3 position = local_to_global(glm::ivec3(lx, ly, lz), chunk_index);
          Block*     block    = world.get_block(position);
          if(block->id == Block::ID_NONE)
            continue;

          for(int i=0; i<std::size(DIRECTIONS); ++i)
          {
            glm::ivec3 direction          = DIRECTIONS[i];
            glm::ivec3 neighbour_position = position + direction;
            Block*     neighbour_block    = world.get_block(neighbour_position);
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

            const BlockData& block_data = world.dimension.block_datas.at(block->id);
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

    chunk.mesh = std::make_unique<graphics::Mesh>(
      std::move(layout),
      graphics::as_bytes(indices),
      graphics::as_bytes(vertices)
    );
  }
};

std::unique_ptr<System> create_chunk_mesher_system()
{
  return std::make_unique<ChunkMesherSystem>();
}

