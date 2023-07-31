#ifndef WORLD_HPP
#define WORLD_HPP

#include <camera.hpp>

#include <types/entity.hpp>
#include <types/dimension.hpp>

struct World
{
  Camera    camera;
  Entity    player;
  Dimension dimension;
};

#endif // WORLD_HPP
