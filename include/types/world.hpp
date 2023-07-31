#ifndef WORLD_HPP
#define WORLD_HPP

#include <camera.hpp>
#include <types/dimension.hpp>

/**********
 * Entity *
 **********/
struct Entity
{
  Transform transform;

  glm::vec3 velocity;
  glm::vec3 bounding_box;
};

/*********
 * World *
 *********/
struct World
{
  Camera    camera;
  Entity    player;
  Dimension dimension;
};

#endif // WORLD_HPP
