#ifndef SYSTEM_HPP
#define SYSTEM_HPP

struct Application;
struct World;
class System
{
public:
  virtual void on_glfw_key_callback(World& world, int key, int scancode, int action, int mods) {}
  virtual void on_glfw_cursor_pos_callback(World& world, double xpos, double ypos)             {}
  virtual void on_glfw_mouse_button_callback(World& world, int button, int action, int mods)   {}
  virtual void on_glfw_scroll_callback(World& world, double xoffset, double yoffset)           {}

public:
  virtual void on_update(Application& application, World& world, float dt)        {};
  virtual void on_render(Application& application, const World& world)            {};
  virtual ~System() = default;
};

#endif // SYSTEM_HPP
