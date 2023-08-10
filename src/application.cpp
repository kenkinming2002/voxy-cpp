#include <application.hpp>

#include <system/chunk_generator.hpp>
#include <system/chunk_mesher.hpp>
#include <system/chunk_renderer.hpp>

#include <system/light.hpp>
#include <system/physics.hpp>

#include <system/player_control.hpp>
#include <system/player_ui.hpp>

#include <system/camera_follow.hpp>

#include <system/debug.hpp>

#include <spdlog/spdlog.h>

static constexpr size_t SEED = 0b1011011010110101110110110101110101011010110101011111010100011010;

Application::Application() :
  m_running(true),
  m_window("voxy", 1024, 720),
  m_timer(),
  m_world{
    .seed = SEED,
    .camera = {
      .aspect = 1024.0f / 720.0f,
      .fovy   = 45.0f,
    },
    .player = {
      .transform = {
        .position = glm::vec3(0.0f, 0.0f, 50.0f),
        .rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
      },
      .velocity     = glm::vec3(0.0f, 0.0f, 0.0f),
      .bounding_box = glm::vec3(0.9f, 0.9f, 1.9f),
    },
    .dimension = {
      .blocks_texture_array = TextureArray({
          "assets/stone.png",
          "assets/grass_bottom.png",
          "assets/grass_side.png",
          "assets/grass_top.png",
          }),
      .block_datas = {
        { .texture_indices = {0, 0, 0, 0, 0, 0} },
        { .texture_indices = {2, 2, 2, 2, 1, 3} },
      },
    },
  }
{
  m_systems.push_back(create_chunk_generator_system());
  m_systems.push_back(create_chunk_mesher_system());
  m_systems.push_back(create_chunk_renderer_system());
  m_systems.push_back(create_light_system());
  m_systems.push_back(create_physics_system());
  m_systems.push_back(create_player_control_system());
  m_systems.push_back(create_player_ui_system());
  m_systems.push_back(create_camera_follow_system());
  m_systems.push_back(create_debug_system());
}

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
    for(auto& system : m_systems)
      system->on_event(m_world, *event);
  }

  // 2: Update
  float dt = m_timer.tick();
  for(auto& system : m_systems)
    system->on_update(m_world, dt);

  // 3: Render
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  for(auto& system : m_systems)
    system->on_render(m_world);

  m_window.swap_buffer();
}

