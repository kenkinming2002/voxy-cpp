#ifndef GLFW_HPP
#define GLFW_HPP

#include <GLFW/glfw3.h>

namespace glfw
{
  struct Context
  {
    Context();
    ~Context();
  };

  struct Window
  {
    Window(const char *name, unsigned width, unsigned height);
    ~Window();

    operator GLFWwindow *() { return window; }
    GLFWwindow *window;
  };
}

#endif // GLFW_HPP
