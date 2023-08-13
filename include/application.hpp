#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Application
{
public:
  static constexpr float FIXED_DT = 1.0f / 20.0f;

public:
  Application();
  virtual ~Application();

public:
  void run();

public:
  void glfw_get_framebuffer_size(int& width, int& height);

public:
  int glfw_get_key(int key);
  int glfw_get_mouse_button(int button);
  void glfw_get_cursor_pos(double& xpos, double& ypos);

public:
  virtual void on_update(float dt) = 0;
  virtual void on_render()         = 0;

private:
  GLFWwindow* m_window;

  double  m_previous_time;
  double  m_accumulated_dt;
};

#endif // APPLICATION_HPP
