#pragma once

#include <world.hpp>

#include <glm/glm.hpp>

struct RayCastBlocksResult
{
  enum class Type { INSIDE_BLOCK, HIT, NONE } type;
  glm::ivec3 position;
  glm::ivec3 normal;
};
RayCastBlocksResult ray_cast_blocks(const World& world, glm::vec3 position, glm::vec3 direction, float length);
