#include <chunk.hpp>
#include <chunk_coords.hpp>

#include <dimension.hpp>

#include <SDL.h>

#include <glm/gtx/norm.hpp>

// At most 1 remash every 5 seconds
static constexpr float REMASH_THROTTLE = 5.0f;

Chunk::Chunk() :
  mesh_invalidated_major(false),
  last_remash_tick(SDL_GetTicks())
{}

void Chunk::update(glm::ivec2 chunk_position, const Dimension& dimension, const std::vector<BlockData>& block_datas)
{
  uint32_t tick = SDL_GetTicks();
  if(mesh_invalidated_major || (mesh_invalidated_minor && (tick - last_remash_tick) / 1000.0f >= REMASH_THROTTLE))
  {
    mesh_invalidated_major = false;
    mesh_invalidated_minor = false;
    last_remash_tick = tick;
    remash(chunk_position, dimension, block_datas);
  }
}

std::optional<Block> Chunk::get_block(glm::ivec3 position) const
{
  if(!data)                                        return std::nullopt;
  if(position.x < 0 || position.x >= CHUNK_WIDTH)  return std::nullopt;
  if(position.y < 0 || position.y >= CHUNK_WIDTH)  return std::nullopt;
  if(position.z < 0 || position.z >= CHUNK_HEIGHT) return std::nullopt;

  return data->blocks[position.z][position.y][position.x];
}

bool Chunk::set_block(glm::ivec3 position, Block block)
{
  if(!data)                                        return false;
  if(position.x < 0 || position.x >= CHUNK_WIDTH)  return false;
  if(position.y < 0 || position.y >= CHUNK_WIDTH)  return false;
  if(position.z < 0 || position.z >= CHUNK_HEIGHT) return false;

  data->blocks[position.z][position.y][position.x] = block;
  return true;
}

void Chunk::explode(glm::vec3 center, float radius)
{
  // TODO: Culling
  glm::ivec3 corner1 = glm::floor(center - glm::vec3(radius, radius, radius));
  glm::ivec3 corner2 = glm::ceil (center + glm::vec3(radius, radius, radius));
  for(int z = corner1.z; z<=corner2.z; ++z)
    for(int y = corner1.y; y<=corner2.y; ++y)
      for(int x = corner1.x; x<=corner2.x; ++x)
      {
        glm::ivec3 pos = { x, y, z };
        if(glm::length2(glm::vec3(pos) - center) < radius * radius)
          set_block(pos, Block{ .presence = false });
      }
}

void Chunk::remash(glm::ivec2 chunk_position, const Dimension& dimension, const std::vector<BlockData>& block_datas)
{
  if(!data)
    return;

  if(mesh)
    mesh.reset();

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
        glm::ivec3 position = local_to_global(glm::ivec3(lx, ly, lz), chunk_position);
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

          const BlockData& block_data = block_datas.at(block.id);
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

  this->mesh = std::make_unique<Mesh>(
      std::move(layout),
      as_bytes(indices),
      as_bytes(vertices)
  );
}

void Chunk::major_invalidate_mesh() { mesh_invalidated_major = true; }
void Chunk::minor_invalidate_mesh() { mesh_invalidated_minor = true; }
