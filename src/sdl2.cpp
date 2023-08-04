#include <sdl2.hpp>

#include <stdexcept>

#include <spdlog/spdlog.h>

namespace sdl2
{
  Context::Context()
  {
    if(SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
      spdlog::error("Failed to initialize SDL2 {}", SDL_GetError());
      throw std::runtime_error("Failed to initialize SDL2");
    }
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

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
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



