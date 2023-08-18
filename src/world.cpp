#include <world.hpp>

#include <coordinates.hpp>

/**********
 * Entity *
 **********/
void entity_apply_force(Entity& entity, glm::vec3 force, float dt)
{
  entity.velocity += dt * force;
}

void entity_apply_impulse(Entity& entity, glm::vec3 force)
{
  entity.velocity += force;
}

/******************
 * Block Accessor *
 ******************/
Block* get_block(Chunk& chunk, glm::ivec3 position)
{
  if(position.x < 0 || position.x >= CHUNK_WIDTH)  return nullptr;
  if(position.y < 0 || position.y >= CHUNK_WIDTH)  return nullptr;
  if(position.z < 0 || position.z >= CHUNK_HEIGHT) return nullptr;

  return &chunk.blocks[position.z][position.y][position.x];
}

const Block* get_block(const Chunk& chunk, glm::ivec3 position)
{
  if(position.x < 0 || position.x >= CHUNK_WIDTH)  return nullptr;
  if(position.y < 0 || position.y >= CHUNK_WIDTH)  return nullptr;
  if(position.z < 0 || position.z >= CHUNK_HEIGHT) return nullptr;

  return &chunk.blocks[position.z][position.y][position.x];
}

Block* get_block(Dimension& dimension, glm::ivec3 position)
{
  auto [local_position, chunk_index] = coordinates::split(position);
  auto it = dimension.chunks.find(chunk_index);
  if(it == dimension.chunks.end())
    return nullptr;

  return ::get_block(it->second, local_position);
}

const Block* get_block(const Dimension& dimension, glm::ivec3 position)
{
  auto [local_position, chunk_index] = coordinates::split(position);
  auto it = dimension.chunks.find(chunk_index);
  if(it == dimension.chunks.end())
    return nullptr;

  return ::get_block(it->second, local_position);
}

Block* get_block(World& world, glm::ivec3 position)
{
  return get_block(world.dimension, position);
}

const Block* get_block(const World& world, glm::ivec3 position)
{
  return get_block(world.dimension, position);
}

/************************
 * Explode the World!!! *
 ************************/
void explode(Chunk& chunk, glm::vec3 center, float radius)
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
          if(Block* block = get_block(chunk, pos))
            block->id = BLOCK_ID_NONE;
      }
}

/**************************
 * Invalidate them ALL!!! *
 **************************/
void invalidate_mesh(Chunk& chunk)
{
  chunk.mesh_invalidated = true;
}

void invalidate_light(Chunk& chunk, glm::ivec3 position)
{
  chunk.light_invalidations.insert(position);
}

void invalidate_mesh(Dimension& dimension, glm::ivec3 position)
{
  auto [local_position, chunk_index] = coordinates::split(position);
  if(auto it = dimension.chunks.find(chunk_index); it != dimension.chunks.end())
    invalidate_mesh(it->second);
}

void invalidate_light(Dimension& dimension, glm::ivec3 position)
{
  auto [local_position, chunk_index] = coordinates::split(position);
  if(auto it = dimension.chunks.find(chunk_index); it != dimension.chunks.end())
    invalidate_light(it->second, local_position);
}

void invalidate_mesh(World& world, glm::ivec3 position)
{
  invalidate_mesh(world.dimension, position);
}

void invalidate_light(World& world, glm::ivec3 position)
{
  invalidate_light(world.dimension, position);
}

