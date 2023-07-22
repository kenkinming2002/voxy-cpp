#include <sdl2.hpp>

#include <stdexcept>

namespace sdl2
{
  Context::Context()
  {
    if(!SDL_Init(SDL_INIT_EVERYTHING))
      throw std::runtime_error("Failed to initialize GLFW");
  }

  Context::~Context()
  {
    SDL_Quit();
  }

  Window::Window(const char *name, unsigned width, unsigned height)
  {
    window = SDL_CreateWindow(
        name,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        width,
        height,
        SDL_WINDOW_OPENGL
    );
    if(!window)
      throw std::runtime_error("Failed to create window\n");

    context = SDL_GL_CreateContext(window);
    if(!context) {
      SDL_DestroyWindow(window);
      throw std::runtime_error("Failed to create OpenGL Context\n");
    }
  }

  Window::~Window()
  {
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
  }
}



