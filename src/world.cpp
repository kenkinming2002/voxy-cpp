#include <world.hpp>

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
