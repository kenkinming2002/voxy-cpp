#include <application.hpp>

#include <spdlog/spdlog.h>
#include <glad/glad.h>
#include <cstdlib>

static constexpr const char* WINDOW_NAME   = "voxy";
static constexpr unsigned    WINDOW_WIDTH  = 1024;
static constexpr unsigned    WINDOW_HEIGHT = 720;

static void GLAPIENTRY message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
  fprintf(stderr, "OpenGL Error: type = %u: %s\n", type, message);
}

Application::Application()
{
  if(SDL_Init(SDL_INIT_EVERYTHING) != 0)
  {
    spdlog::error("Failed to initialize SDL2 {}", SDL_GetError());
    std::exit(-1);
  }

  m_window = SDL_CreateWindow(WINDOW_NAME, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);
  if(!m_window)
  {
    spdlog::error("Failed to create window {}", SDL_GetError());
    std::exit(-1);
  }
  SDL_SetRelativeMouseMode(SDL_TRUE);

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
  m_gl_context = SDL_GL_CreateContext(m_window);
  if(!m_gl_context)
  {
    spdlog::error("Failed to create OpenGL Context {}", SDL_GetError());
    std::exit(-1);
  }

  if(!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
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

  m_previous_ticks = SDL_GetTicks();
}

Application::~Application()
{
  SDL_GL_DeleteContext(m_gl_context);
  SDL_DestroyWindow(m_window);
}

void Application::run()
{
  for(;;)
  {
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
      if(event.type == SDL_QUIT)
        return;
      this->on_event(event);
    }

    Uint32 ticks = SDL_GetTicks();
    float dt = (ticks - m_previous_ticks) / 1000.0f;
    m_previous_ticks = ticks;

    this->on_update(dt);
    this->on_render();
    SDL_GL_SwapWindow(m_window);
  }
}

