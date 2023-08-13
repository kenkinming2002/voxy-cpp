#include <application.hpp>

#include <spdlog/spdlog.h>
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
  m_window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_NAME, nullptr, nullptr);
  if(!m_window)
  {
    const char *description;
    glfwGetError(&description);
    spdlog::error("Failed to initialize GLFW {}", description);
    std::exit(-1);
  }

  glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

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

  m_previous_time = glfwGetTime();
}

Application::~Application()
{
  glfwDestroyWindow(m_window);
}

void Application::run()
{
  for(;;)
  {
    glfwPollEvents();
    if(glfwWindowShouldClose(m_window))
      return;

    double time = glfwGetTime();
    m_accumulated_dt += time - m_previous_time;
    m_previous_time = time;

    if(m_accumulated_dt >= FIXED_DT)
    {
      m_accumulated_dt -= FIXED_DT;
      this->on_update(FIXED_DT);
    }

    this->on_render();
    glfwSwapBuffers(m_window);
  }
}

int Application::glfw_get_key(int key)
{
  return glfwGetKey(m_window, key);
}

int Application::glfw_get_mouse_button(int button)
{
  return glfwGetMouseButton(m_window, button);
}

void Application::glfw_get_cursor_pos(double& xpos, double& ypos)
{
  glfwGetCursorPos(m_window, &xpos, &ypos);
}

