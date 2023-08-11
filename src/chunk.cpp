#include <chunk.hpp>

#include <glm/gtx/norm.hpp>

Block* Chunk::get_block(glm::ivec3 position)
{
  if(position.x < 0 || position.x >= Chunk::WIDTH)  return nullptr;
  if(position.y < 0 || position.y >= Chunk::WIDTH)  return nullptr;
  if(position.z < 0 || position.z >= Chunk::HEIGHT) return nullptr;

  return &blocks[position.z][position.y][position.x];
}

const Block* Chunk::get_block(glm::ivec3 position) const
{
  if(position.x < 0 || position.x >= Chunk::WIDTH)  return nullptr;
  if(position.y < 0 || position.y >= Chunk::WIDTH)  return nullptr;
  if(position.z < 0 || position.z >= Chunk::HEIGHT) return nullptr;

  return &blocks[position.z][position.y][position.x];
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
          if(Block* block = get_block(pos))
            block->id = Block::ID_NONE;
      }
}

void Chunk::major_invalidate_mesh() { mesh_invalidated_major = true; }
void Chunk::minor_invalidate_mesh() { mesh_invalidated_minor = true; }

void Chunk::lighting_invalidate(glm::ivec3 position)
{
  pending_lighting_updates.insert(position);
}
