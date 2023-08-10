#include <world.hpp>

/*********
 * Chunk *
 *********/
Block* Chunk::get_block(glm::ivec3 position)
{
  if(!data)                                         return nullptr;
  if(position.x < 0 || position.x >= Chunk::WIDTH)  return nullptr;
  if(position.y < 0 || position.y >= Chunk::WIDTH)  return nullptr;
  if(position.z < 0 || position.z >= Chunk::HEIGHT) return nullptr;

  return &data->blocks[position.z][position.y][position.x];
}

const Block* Chunk::get_block(glm::ivec3 position) const
{
  if(!data)                                         return nullptr;
  if(position.x < 0 || position.x >= Chunk::WIDTH)  return nullptr;
  if(position.y < 0 || position.y >= Chunk::WIDTH)  return nullptr;
  if(position.z < 0 || position.z >= Chunk::HEIGHT) return nullptr;

  return &data->blocks[position.z][position.y][position.x];
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

/*************
 * Dimension *
 *************/
int modulo(int a, int b)
{
  int value = a % b;
  if(value < 0)
    value += b;

  assert(0 <= value);
  assert(value < b);
  return value;
}

Block* Dimension::get_block(glm::ivec3 position)
{
  // FIXME: Refactor me`
  glm::ivec3 local_position = {
    modulo(position.x, Chunk::WIDTH),
    modulo(position.y, Chunk::WIDTH),
    position.z
  };

  glm::ivec2 chunk_index = {
    (position.x - local_position.x) / Chunk::WIDTH,
    (position.y - local_position.y) / Chunk::WIDTH,
  };

  auto it = chunks.find(chunk_index);
  if(it == chunks.end())
    return nullptr;

  return it->second.get_block(local_position);
}

const Block* Dimension::get_block(glm::ivec3 position) const
{
  // FIXME: Refactor me`
  glm::ivec3 local_position = {
    modulo(position.x, Chunk::WIDTH),
    modulo(position.y, Chunk::WIDTH),
    position.z
  };

  glm::ivec2 chunk_index = {
    (position.x - local_position.x) / Chunk::WIDTH,
    (position.y - local_position.y) / Chunk::WIDTH,
  };

  auto it = chunks.find(chunk_index);
  if(it == chunks.end())
    return nullptr;

  return it->second.get_block(local_position);
}

void Dimension::major_invalidate_mesh(glm::ivec3 position)
{
  glm::ivec3 local_position = {
    modulo(position.x, Chunk::WIDTH),
    modulo(position.y, Chunk::WIDTH),
    position.z
  };
  glm::ivec2 chunk_index = {
    (position.x - local_position.x) / Chunk::WIDTH,
    (position.y - local_position.y) / Chunk::WIDTH,
  };
  chunks[chunk_index].major_invalidate_mesh();
}

void Dimension::minor_invalidate_mesh(glm::ivec3 position)
{
  glm::ivec3 local_position = {
    modulo(position.x, Chunk::WIDTH),
    modulo(position.y, Chunk::WIDTH),
    position.z
  };
  glm::ivec2 chunk_index = {
    (position.x - local_position.x) / Chunk::WIDTH,
    (position.y - local_position.y) / Chunk::WIDTH,
  };
  chunks[chunk_index].minor_invalidate_mesh();
}

void Dimension::lighting_invalidate(glm::ivec3 position)
{
  pending_lighting_updates.insert(position);
}

Block* World::get_block(glm::ivec3 position)
{
  return dimension.get_block(position);
}

const Block* World::get_block(glm::ivec3 position) const
{
  return dimension.get_block(position);
}

void World::invalidate_mesh_major(glm::ivec3 position)
{
  dimension.major_invalidate_mesh(position);
}

void World::invalidate_mesh_minor(glm::ivec3 position)
{
  dimension.minor_invalidate_mesh(position);
}

void World::invalidate_light(glm::ivec3 position)
{
  dimension.lighting_invalidate(position);
}
