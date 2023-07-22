#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/glm.hpp>

struct Camera
{
  static constexpr float MOVEMENT_SPEED = 2.5f;
  static constexpr float ROTATION_SPEED = 0.1f;

  glm::vec3 position = glm::vec3(0.0f, 0.0f,  3.0f);
  float yaw          = -90.0f;
  float pitch        = 0.0f;
  float fov          = 45.0f;

  glm::vec3 up() const;
  glm::vec3 forward() const;
  glm::vec3 right() const;

  glm::mat4 view() const;
  glm::mat4 projection() const;

  void rotate(float x, float y);
  void translate(float x, float y);
  void zoom(float factor);
};


#endif // CAMERA_HPP
