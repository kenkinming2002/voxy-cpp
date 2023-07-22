#ifndef GLFW_HPP
#define GLFW_HPP

#include <SDL2/SDL.h>

namespace sdl2
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

    operator SDL_Window *() { return window; }

    SDL_Window    *window;
    SDL_GLContext  context;
  };
}

#endif // GLFW_HPP
