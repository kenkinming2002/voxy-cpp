#include <glfw.hpp>

#include <stdexcept>

namespace glfw
{
  Context::Context()
  {
    if(!glfwInit())
      throw std::runtime_error("Failed to initialize GLFW");
  }

  Context::~Context()
  {
    glfwTerminate();
  }

  Window::Window(const char *name, unsigned width, unsigned height)
  {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    window = glfwCreateWindow(width, height, name, nullptr, nullptr);
    if(!window)
      throw std::runtime_error("Failed to create window\n");
  }

  Window::~Window()
  {
    glfwDestroyWindow(window);
  }
}



