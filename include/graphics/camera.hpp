#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <transform.hpp>

#include <glm/glm.hpp>

namespace graphics
{
  struct Camera
  {
    Transform transform;

    float aspect;
    float fovy;

    glm::mat4 view() const;
    glm::mat4 projection() const;

    void zoom(float factor);
  };
}

#endif // CAMERA_HPP
