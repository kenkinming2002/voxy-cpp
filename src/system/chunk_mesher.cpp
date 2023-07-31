#include <system/chunk_mesher.hpp>

#include <dimension.hpp>
#include <chunk_coords.hpp>

#include <SDL.h>

#include <spdlog/spdlog.h>

#include <glm/glm.hpp>

static constexpr float REMASH_THROTTLE = 5.0f;

class ChunkMesherSystemImpl : public ChunkMesherSystem
{
private:
  void update_chunk(Dimension& dimension, glm::ivec2 chunk_index) override
  {
    Chunk& chunk = dimension.get_chunk(chunk_index);

    uint32_t tick = SDL_GetTicks();
    if(!chunk.mesh || chunk.mesh_invalidated_major || (chunk.mesh_invalidated_minor && (tick - chunk.last_remash_tick) / 1000.0f >= REMASH_THROTTLE))
    {
      chunk.mesh_invalidated_major = false;
      chunk.mesh_invalidated_minor = false;
      chunk.last_remash_tick = tick;
      remesh_chunk(dimension, chunk_index);
    }
  }

  void remesh_chunk(Dimension& dimension, glm::ivec2 chunk_index) override
  {
    Chunk& chunk = dimension.get_chunk(chunk_index);
    if(!chunk.data)
    {
      spdlog::warn("Chunk at {}, {} has not yet been generated", chunk_index.x, chunk_index.y);
      return;
    }

    if(chunk.mesh)
      chunk.mesh.reset();

    struct Vertex
    {
      glm::vec3 position;
      glm::vec2 texture_coords;
      uint32_t  texture_index;
      float     light_level;
    };

    std::vector<uint32_t> indices;
    std::vector<Vertex>   vertices;
    for(int lz=0; lz<CHUNK_HEIGHT; ++lz)
      for(int ly=0; ly<CHUNK_WIDTH; ++ly)
        for(int lx=0; lx<CHUNK_WIDTH; ++lx)
        {
          glm::ivec3 position = local_to_global(glm::ivec3(lx, ly, lz), chunk_index);
          Block      block    = dimension.get_block(position).value();
          if(!block.presence)
            continue;

          for(int i=0; i<std::size(DIRECTIONS); ++i)
          {
            glm::ivec3 direction          = DIRECTIONS[i];
            glm::ivec3 neighbour_position = position + direction;
            Block      neighbour_block    = dimension.get_block(neighbour_position).value_or(Block{.presence = 0, .light_level = 15});
            if(neighbour_block.presence)
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

            const BlockData& block_data = dimension.block_datas().at(block.id);
            vertices.push_back(Vertex{ .position = center + ( - 0.5f * glm::vec3(right) - 0.5f * glm::vec3(up)), .texture_coords = {0.0f, 0.0f}, .texture_index = block_data.texture_indices[i], .light_level = neighbour_block.light_level / 16.0f, });
            vertices.push_back(Vertex{ .position = center + ( + 0.5f * glm::vec3(right) - 0.5f * glm::vec3(up)), .texture_coords = {1.0f, 0.0f}, .texture_index = block_data.texture_indices[i], .light_level = neighbour_block.light_level / 16.0f, });
            vertices.push_back(Vertex{ .position = center + ( - 0.5f * glm::vec3(right) + 0.5f * glm::vec3(up)), .texture_coords = {0.0f, 1.0f}, .texture_index = block_data.texture_indices[i], .light_level = neighbour_block.light_level / 16.0f, });
            vertices.push_back(Vertex{ .position = center + ( + 0.5f * glm::vec3(right) + 0.5f * glm::vec3(up)), .texture_coords = {1.0f, 1.0f}, .texture_index = block_data.texture_indices[i], .light_level = neighbour_block.light_level / 16.0f, });
            // NOTE: Brackets added so that it is possible for the compiler to do constant folding if loop is unrolled, not that it would actually do it.
          }
        }

    MeshLayout layout{
      .index_type = IndexType::UNSIGNED_INT,
        .stride = sizeof(Vertex),
        .attributes = {
          { .type = AttributeType::FLOAT3,        .offset = offsetof(Vertex, position),       },
          { .type = AttributeType::FLOAT2,        .offset = offsetof(Vertex, texture_coords), },
          { .type = AttributeType::UNSIGNED_INT1, .offset = offsetof(Vertex, texture_index),  },
          { .type = AttributeType::FLOAT1,        .offset = offsetof(Vertex, light_level),    },
        },
    };

    chunk.mesh = std::make_unique<Mesh>(
      std::move(layout),
      as_bytes(indices),
      as_bytes(vertices)
    );
  }
};

std::unique_ptr<ChunkMesherSystem> ChunkMesherSystem::create()
{
  return std::make_unique<ChunkMesherSystemImpl>();
}

