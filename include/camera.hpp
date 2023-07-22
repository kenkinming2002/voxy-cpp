#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/glm.hpp>

struct Camera
{
  static constexpr float MOVEMENT_SPEED = 50.0f;
  static constexpr float ROTATION_SPEED = 0.1f;

  glm::vec3 position = glm::vec3(-5.0f, -5.0f,  50.0f);
  float aspect       = 1024.0f / 720.0f;
  float yaw          = 45.0f;
  float pitch        = -45.0f;
  float fov          = 45.0f;

  glm::vec3 up() const;
  glm::vec3 forward() const;
  glm::vec3 right() const;

  glm::mat4 view() const;
  glm::mat4 projection() const;

  void rotate(float x, float y);
  void translate(float x, float y, float z);
  void zoom(float factor);
};


#endif // CAMERA_HPP
