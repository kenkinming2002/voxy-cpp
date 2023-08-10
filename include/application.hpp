#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <SDL.h>

class Application
{
public:
  Application();
  virtual ~Application();

public:
  void run();

public:
  virtual void on_event(SDL_Event event) = 0;
  virtual void on_update(float dt)       = 0;
  virtual void on_render()               = 0;

private:
  SDL_Window*   m_window;
  SDL_GLContext m_gl_context;
  Uint32        m_previous_ticks;
};

#endif // APPLICATION_HPP
