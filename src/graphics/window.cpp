#include <graphics/window.hpp>

#include <spdlog/spdlog.h>

namespace graphics
{
  static void GLAPIENTRY message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
  {
    fprintf(stderr, "OpenGL Error: type = %u: %s\n", type, message);
  }

  Window::Window(const char *title, unsigned width, unsigned height)
  {
    if(!glfwInit())
    {
      const char *description;
      glfwGetError(&description);
      spdlog::error("Failed to initialize GLFW {}", description);
      std::exit(-1);
    }

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    m_window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if(!m_window)
    {
      const char *description;
      glfwGetError(&description);
      spdlog::error("Failed to initialize GLFW {}", description);
      std::exit(-1);
    }

    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwSetWindowUserPointer(m_window, this);
    glfwSetKeyCallback(m_window, &Window::glfw_key_callback);

    glfwMakeContextCurrent(m_window);
    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
      spdlog::error("Failed to create Load OpenGL functions");
      std::exit(-1);
    }

    glDebugMessageCallback(message_callback, 0);

    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_PACK_ALIGNMENT,   1);
  }

  void Window::poll_events()
  {
    glfwPollEvents();
  }

  bool Window::should_close()
  {
    return glfwWindowShouldClose(m_window);
  }

  void Window::swap_buffers()
  {
    glfwSwapBuffers(m_window);
  }

  void Window::get_framebuffer_size(int& width, int& height)
  {
    glfwGetFramebufferSize(m_window, &width, &height);
  }

  int Window::get_key(int key)
  {
    return glfwGetKey(m_window, key);
  }

  int Window::get_mouse_button(int button)
  {
    return glfwGetMouseButton(m_window, button);
  }

  void Window::get_cursor_pos(double& xpos, double& ypos)
  {
    glfwGetCursorPos(m_window, &xpos, &ypos);
  }

  void Window::glfw_on_key(std::function<void(int key, int scancode, int action, int mods)> callback)
  {
    m_glfw_on_key = std::move(callback);
  }

  void Window::glfw_key_callback(GLFWwindow* glfw_window, int key, int scancode, int action, int mods)
  {
    Window& window = *(Window*)glfwGetWindowUserPointer(glfw_window);
    if(window.m_glfw_on_key)
      window.m_glfw_on_key(key, scancode, action, mods);
  }
}
