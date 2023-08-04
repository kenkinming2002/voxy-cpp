#include <window.hpp>

#include <glad/glad.h>

#include <stdexcept>

static void GLAPIENTRY message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
  fprintf(stderr, "OpenGL Error: type = %u: %s\n", type, message);
}

Window::Window(const char *name, unsigned width, unsigned height) : m_context(), m_window(name, width, height)
{
  SDL_SetRelativeMouseMode(SDL_TRUE);
  if(!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
    throw std::runtime_error("Failed to load OpenGL functions with GLAD");

  glDebugMessageCallback(message_callback, 0);

  glEnable(GL_LINE_SMOOTH);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glEnable(GL_BLEND);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glPixelStorei(GL_PACK_ALIGNMENT,   1);
}

std::optional<SDL_Event> Window::poll_event()
{
  SDL_Event event;
  if(!SDL_PollEvent(&event))
    return std::nullopt;

  return event;
}

void Window::swap_buffer()
{
  SDL_GL_SwapWindow(m_window);
}
