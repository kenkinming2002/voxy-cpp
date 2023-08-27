#include <graphics/camera.hpp>

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>

namespace graphics
{
  glm::mat4 Camera::view() const
  {
    return glm::inverse(transform.as_matrix());
  }

  glm::mat4 Camera::projection() const
  {
    glm::mat4 axis_adjust = glm::mat4(0.0f);
    axis_adjust[0][0] =  1.0f; // our x-axis goes to +ve x-axis in OpenGL
    axis_adjust[1][2] = -1.0f; // our y-axis goes to -ve z-axis in OpenGL
    axis_adjust[2][1] =  1.0f; // our z-axis goes to +ve y-axis in OpenGL
    axis_adjust[3][3] =  1.0f; // Keep w-component
    return glm::perspective(glm::radians(fovy), aspect, 0.1f, 500.0f) * axis_adjust;
  }

  void Camera::zoom(float factor)
  {
    fovy += factor;
    fovy = std::clamp(fovy, 1.0f, 45.0f);
  }
}

