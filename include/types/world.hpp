#ifndef WORLD_HPP
#define WORLD_HPP

#include <camera.hpp>

#include <types/entity.hpp>
#include <types/dimension.hpp>

#include <optional>

struct World
{
  Camera    camera;
  Entity    player;
  Dimension dimension;

  std::optional<glm::ivec3> selection;
};

#endif // WORLD_HPP
