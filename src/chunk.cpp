#include <chunk.hpp>

static constexpr glm::ivec3 DIRECTIONS[] = {
  {-1, 0, 0},
  { 1, 0, 0},
  {0, -1, 0},
  {0,  1, 0},
  {0, 0, -1},
  {0, 0,  1},
};

struct Vertex
{
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec3 color;
};

static Mesh generate_blocks_mesh(glm::ivec2 cpos, const Blocks& blocks)
{
  std::vector<uint32_t> indices;
  std::vector<Vertex>   vertices;
  for(int lz=0; lz<blocks.height(); ++lz)
    for(int ly=0; ly<blocks.width(); ++ly)
      for(int lx=0; lx<blocks.width(); ++lx)
      {
        glm::ivec3 lpos  = { lx, ly, lz };
        Block      block = blocks.get(lpos);
        if(!block.presence)
          continue;

        for(glm::ivec3 dir : DIRECTIONS)
        {
          glm::ivec3 neighbour_lpos  = lpos + dir;
          Block      neighbour_block = blocks.get(neighbour_lpos);
          if(neighbour_block.presence)
            continue;

          uint32_t index_base = vertices.size();
          indices.push_back(index_base + 0);
          indices.push_back(index_base + 1);
          indices.push_back(index_base + 2);
          indices.push_back(index_base + 2);
          indices.push_back(index_base + 1);
          indices.push_back(index_base + 3);

          glm::ivec3 out   = dir;
          glm::ivec3 up    = dir.z == 0.0 ? glm::ivec3(0, 0, 1) : glm::ivec3(1, 0, 0);
          glm::ivec3 right = glm::cross(glm::vec3(up), glm::vec3(out));

          glm::vec3 center = glm::vec3(lpos) + 0.5f * glm::vec3(out);
          vertices.push_back(Vertex{ .position = center + ( - 0.5f * glm::vec3(right) - 0.5f * glm::vec3(up)), .normal = dir, .color = block.color });
          vertices.push_back(Vertex{ .position = center + ( + 0.5f * glm::vec3(right) - 0.5f * glm::vec3(up)), .normal = dir, .color = block.color });
          vertices.push_back(Vertex{ .position = center + ( - 0.5f * glm::vec3(right) + 0.5f * glm::vec3(up)), .normal = dir, .color = block.color });
          vertices.push_back(Vertex{ .position = center + ( + 0.5f * glm::vec3(right) + 0.5f * glm::vec3(up)), .normal = dir, .color = block.color });
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

Chunk::Chunk(size_t seed, glm::ivec2 cpos) :
  blocks(seed, cpos),
  blocks_mesh(generate_blocks_mesh(cpos, blocks)),
  blocks_need_remash(false)
{}
