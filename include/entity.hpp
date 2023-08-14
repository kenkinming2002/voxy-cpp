#pragma once

#include <transform.hpp>

#include <glm/glm.hpp>

struct EntityData
{
public:
  Transform transform;

  glm::vec3 velocity;
  glm::vec3 bounding_box;

  glm::vec3 eye_offset;

  bool collided;
  bool grounded;

public:
  void apply_force(glm::vec3 force, float dt)
  {
    velocity += dt * force;
  }

  void apply_impulse(glm::vec3 force)
  {
    velocity += force;
  }
};

