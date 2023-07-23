#include <camera.hpp>

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>

glm::vec3 Camera::up() const
{
  return glm::vec3(0.0f, 0.0f,  1.0f);
}

glm::vec3 Camera::forward() const
{
  glm::vec3 direction;
  direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
  direction.y = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
  direction.z = sin(glm::radians(pitch));
  return direction;
}

glm::vec3 Camera::right() const
{
  return glm::normalize(glm::cross(forward(), up()));
}

glm::mat4 Camera::view() const
{
  return glm::lookAt(position, position + forward(), up());
}

glm::mat4 Camera::projection() const
{
  return glm::perspective(glm::radians(fov), aspect, 0.1f, 500.0f);
}

void Camera::rotate(float x, float y)
{
  yaw   += x * ROTATION_SPEED;
  pitch += y * ROTATION_SPEED;
  pitch = std::clamp(pitch, -89.0f, 89.0f);
}

void Camera::translate(float x, float y, float z)
{
  glm::vec3 offset = MOVEMENT_SPEED * (
      x * right() +
      y * forward() +
      z * up()
  );
  position += offset;
}

void Camera::zoom(float factor)
{
  fov += factor;
  fov = std::clamp(fov, 1.0f, 45.0f);
}
