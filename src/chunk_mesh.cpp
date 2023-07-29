#include <chunk_mesh.hpp>

#include <chunk_manager.hpp>

Mesh generate_chunk_mesh(glm::ivec2 chunk_position, const ChunkData& chunk_data, const std::vector<BlockData>& block_datas)
{
  struct Vertex
  {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
    uint32_t  texture_index;
  };

  std::vector<uint32_t> indices;
  std::vector<Vertex>   vertices;
  for(int lz=0; lz<chunk_data.slices.size(); ++lz)
    for(int ly=0; ly<CHUNK_WIDTH; ++ly)
      for(int lx=0; lx<CHUNK_WIDTH; ++lx)
      {
        glm::ivec3 position  = { lx, ly, lz };
        Block      block     = chunk_data.get_block(position).value();
        if(!block.presence)
          continue;

        for(int i=0; i<std::size(DIRECTIONS); ++i)
        {
          glm::ivec3 direction          = DIRECTIONS[i];
          glm::ivec3 neighbour_position = position + direction;
          Block      neighbour_block    = chunk_data.get_block(neighbour_position).value_or(Block{.presence = 0}); // TODO: Actually lookup block in adjacent chunks
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

          const BlockData& block_data = block_datas.at(block.id);
          vertices.push_back(Vertex{ .position = center + ( - 0.5f * glm::vec3(right) - 0.5f * glm::vec3(up)), .normal = direction, .uv = {0.0f, 0.0f}, .texture_index = block_data.texture_indices[i] });
          vertices.push_back(Vertex{ .position = center + ( + 0.5f * glm::vec3(right) - 0.5f * glm::vec3(up)), .normal = direction, .uv = {1.0f, 0.0f}, .texture_index = block_data.texture_indices[i] });
          vertices.push_back(Vertex{ .position = center + ( - 0.5f * glm::vec3(right) + 0.5f * glm::vec3(up)), .normal = direction, .uv = {0.0f, 1.0f}, .texture_index = block_data.texture_indices[i] });
          vertices.push_back(Vertex{ .position = center + ( + 0.5f * glm::vec3(right) + 0.5f * glm::vec3(up)), .normal = direction, .uv = {1.0f, 1.0f}, .texture_index = block_data.texture_indices[i] });
          // NOTE: Brackets added so that it is possible for the compiler to do constant folding if loop is unrolled, not that it would actually do it.
        }
      }

  MeshLayout layout{
    .index_type = IndexType::UNSIGNED_INT,
    .stride = sizeof(Vertex),
    .attributes = {
      { .type = AttributeType::FLOAT3,        .offset = offsetof(Vertex, position),      },
      { .type = AttributeType::FLOAT3,        .offset = offsetof(Vertex, normal),        },
      { .type = AttributeType::FLOAT2,        .offset = offsetof(Vertex, uv),            },
      { .type = AttributeType::UNSIGNED_INT1, .offset = offsetof(Vertex, texture_index), },
    },
  };

  return Mesh(layout,
    as_bytes(indices),
    as_bytes(vertices)
  );
}

