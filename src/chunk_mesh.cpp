#include <chunk_mesh.hpp>

Mesh generate_chunk_mesh(glm::ivec2 chunk_position, const ChunkData& chunk_data)
{
  struct Vertex
  {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;
  };

  std::vector<uint32_t> indices;
  std::vector<Vertex>   vertices;
  for(int lz=0; lz<chunk_data.slices.size(); ++lz)
    for(int ly=0; ly<CHUNK_WIDTH; ++ly)
      for(int lx=0; lx<CHUNK_WIDTH; ++lx)
      {
        glm::ivec3 position  = { lx, ly, lz };
        Block      block     = get_block(chunk_data, position);
        if(!block.presence)
          continue;

        for(glm::ivec3 direction : DIRECTIONS)
        {
          glm::ivec3 neighbour_position = position + direction;
          Block      neighbour_block    = get_block(chunk_data, neighbour_position);
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

          glm::vec3 color;
          switch(block.id)
          {
          case Block::ID_STONE: color = glm::vec3(0.7, 0.7, 0.7); break;
          case Block::ID_GRASS: color = glm::vec3(0.2, 1.0, 0.2); break;
          default: color = glm::vec3(1.0, 1.0, 1.0); break;
          }

          vertices.push_back(Vertex{ .position = center + ( - 0.5f * glm::vec3(right) - 0.5f * glm::vec3(up)), .normal = direction, .color = color });
          vertices.push_back(Vertex{ .position = center + ( + 0.5f * glm::vec3(right) - 0.5f * glm::vec3(up)), .normal = direction, .color = color });
          vertices.push_back(Vertex{ .position = center + ( - 0.5f * glm::vec3(right) + 0.5f * glm::vec3(up)), .normal = direction, .color = color });
          vertices.push_back(Vertex{ .position = center + ( + 0.5f * glm::vec3(right) + 0.5f * glm::vec3(up)), .normal = direction, .color = color });
          // NOTE: Brackets added so that it is possible for the compiler to do constant folding if loop is unrolled, not that it would actually do it.
        }
      }

  MeshLayout layout{
    .index_type = IndexType::UNSIGNED_INT,
    .stride = sizeof(Vertex),
    .attributes = {
      { .type = AttributeType::FLOAT3, .offset = offsetof(Vertex, position), },
      { .type = AttributeType::FLOAT3, .offset = offsetof(Vertex, normal),   },
      { .type = AttributeType::FLOAT3, .offset = offsetof(Vertex, color),    },
    },
  };

  return Mesh(layout,
    as_bytes(indices),
    as_bytes(vertices)
  );
}

