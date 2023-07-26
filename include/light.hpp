#ifndef LIGHT_HPP
#define LIGHT_HPP

#include <mesh.hpp>

#include <glm/glm.hpp>

struct Light
{
  glm::vec3 position;
  glm::vec3 ambient;
  glm::vec3 diffuse;
};


#endif // LIGHT_HPP
