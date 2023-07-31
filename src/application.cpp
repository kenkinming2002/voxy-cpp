#include <application.hpp>

static constexpr size_t SEED = 0b1011011010110101110110110101110101011010110101011111010100011010;

Application::Application() :
  m_running(true),
  m_window("voxy", 1024, 720),
  m_timer(),
  m_world(),
  m_chunk_generator_system(ChunkGeneratorSystem::create(SEED)),
  m_chunk_mesher_system(ChunkMesherSystem::create()),
  m_chunk_renderer_system(ChunkRendererSystem::create()),
  m_light_system(LightSystem::create()),
  m_physics_system(PhysicsSystem::create()),
  m_player_movement_system(PlayerMovementSystem::create()),
  m_camera_follow_system(CameraFollowSystem::create()),
  m_debug_system(DebugSystem::create())
{}

void Application::run()
{
  while(m_running)
    loop();
}

void Application::loop()
{
  // 1: Handle Events
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
    m_player_movement_system->handle_event(m_world, *event);
  }

  // 2: Update
  float dt = m_timer.tick();
  m_player_movement_system->update(m_world, dt);
  m_light_system->update(m_world);
  m_chunk_generator_system->update(m_world);
  m_chunk_mesher_system->update(m_world);
  m_physics_system->update(m_world, dt);
  m_camera_follow_system->update(m_world, dt);

  // 3: Render
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  m_chunk_renderer_system->render(m_world);
  m_debug_system->render(m_world);
  m_window.swap_buffer();
}

