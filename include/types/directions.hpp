#ifndef TYPES_DIRECTION_HPP
#define TYPES_DIRECTION_HPP

#include <glm/glm.hpp>

static constexpr glm::ivec3 DIRECTIONS[] = {
  {-1, 0, 0},
  { 1, 0, 0},
  {0, -1, 0},
  {0,  1, 0},
  {0, 0, -1},
  {0, 0,  1},
};

#endif // TYPES_DIRECTION_HPP
