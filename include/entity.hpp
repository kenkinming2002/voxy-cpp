#pragma once

#include <transform.hpp>

#include <glm/glm.hpp>

struct Entity
{
  Transform transform;

  glm::vec3 velocity;
  glm::vec3 bounding_box;
  bool      collided;
};

