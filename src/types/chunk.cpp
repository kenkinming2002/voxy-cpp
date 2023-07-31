#include <types/chunk.hpp>
#include <types/chunk_coords.hpp>

#include <SDL.h>

#include <glm/gtx/norm.hpp>

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

void Chunk::major_invalidate_mesh() { mesh_invalidated_major = true; }
void Chunk::minor_invalidate_mesh() { mesh_invalidated_minor = true; }
