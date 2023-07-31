#include <application.hpp>

static constexpr size_t SEED = 0b1011011010110101110110110101110101011010110101011111010100011010;

Application::Application() :
  m_running(true),
  m_window("voxy", 1024, 720),
  m_timer(),
  m_world(SEED) {}

void Application::run()
{
  while(m_running)
    loop();
}

void Application::loop()
{
  while(auto event = m_window.poll_event())
  {
    switch(event->type) {
      case SDL_KEYDOWN:
        if(event->key.keysym.sym == SDLK_ESCAPE)
          m_running = false;
        break;
      case SDL_QUIT:
        m_running = false;
        break;
    }
    m_world.handle_event(*event);
  }

  float dt = m_timer.tick();
  m_world.update(dt);

  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  m_world.render();
  m_window.swap_buffer();
}

