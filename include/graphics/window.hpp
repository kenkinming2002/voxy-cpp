#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <functional>

namespace graphics
{
  class Window
  {
  public:
    Window(const char *title, unsigned width, unsigned height);

  public:
    void poll_events();
    bool should_close();
    void swap_buffers();

  public:
    void get_framebuffer_size(int& width, int& height);
    int get_key(int key);
    int get_mouse_button(int button);
    void get_cursor_pos(double& xpos, double& ypos);

  public:
    void glfw_on_key(std::function<void(int key, int scancode, int action, int mods)> callback);

  private:
    static void glfw_key_callback(GLFWwindow* glfw_window, int key, int scancode, int action, int mods);

  private:
    GLFWwindow*                                                      m_window;
    std::function<void(int key, int scancode, int action, int mods)> m_glfw_on_key;
  };

}
