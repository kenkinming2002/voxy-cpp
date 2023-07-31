#ifndef TYPES_ENTITY_HPP
#define TYPES_ENTITY_HPP

#include <transform.hpp>

#include <glm/glm.hpp>

struct Entity
{
  Transform transform;

  glm::vec3 velocity;
  glm::vec3 bounding_box;
};

#endif // TYPES_ENTITY_HPP
