#include <application.hpp>

#include <world.hpp>
#include <system.hpp>

#include <system/chunk_generator.hpp>
#include <system/chunk_renderer.hpp>

#include <system/light.hpp>
#include <system/physics.hpp>

#include <system/player_control.hpp>
#include <system/player_ui.hpp>

#include <system/camera_follow.hpp>

#include <system/debug.hpp>

#include <spdlog/spdlog.h>

#include <cxxabi.h>

static constexpr size_t SEED = 0b1011011010110101110110110101110101011010110101011111010100011010;

class Voxy : public Application
{
public:
  Voxy();

private:
  void on_update(float dt) override;
  void on_render()         override;

private:
  World                                m_world;
  std::vector<std::unique_ptr<System>> m_systems;
};

Voxy::Voxy() :
  m_world{
    .config = {
      .seed = SEED,
      .terrain = {
        .layers = {
          { .block_id = Block::ID_STONE, .frequency = 0.03f, .amplitude = 40.0f, .lacunarity = 2.0f, .persistence = 0.5f, .octaves = 4, },
          { .block_id = Block::ID_GRASS, .frequency = 0.01f, .amplitude = 5.0f,  .lacunarity = 2.0f, .persistence = 0.5f, .octaves = 2, },
        },
      },
      .caves = {
        .max_per_chunk = 2,
        .max_segment   = 10,
        .step          = 5.0f,
        .min_height    = 10.0f,
        .max_height    = 30.0f,

        .dig_frequency   = 0.1f,
        .dig_amplitude   = 1.0f,
        .dig_lacunarity  = 2.0f,
        .dig_persistence = 0.5f,
        .dig_octaves     = 4,

        .radius             = 2.0f,
        .radius_frequency   = 0.1f,
        .radius_amplitude   = 3.0f,
        .radius_lacunarity  = 2.0f,
        .radius_persistence = 0.5f,
      },
    },
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
    .dimension = {},
  }
{
  m_systems.push_back(create_chunk_generator_system());
  m_systems.push_back(create_chunk_renderer_system());
  m_systems.push_back(create_light_system());
  m_systems.push_back(create_physics_system());
  m_systems.push_back(create_player_control_system());
  m_systems.push_back(create_player_ui_system());
  m_systems.push_back(create_camera_follow_system());
  m_systems.push_back(create_debug_system());
}

void Voxy::on_update(float dt)
{
  for(auto& system : m_systems)
  {
    double time_begin = glfwGetTime();
    system->on_update(*this, m_world, dt);
    double time_end = glfwGetTime();
    double time = time_end - time_begin;

    const std::type_info& type_info = typeid(*system);

    int status;
    const char* type_name = abi::__cxa_demangle(type_info.name(), NULL, NULL, &status);
    if(time <= 25)
      spdlog::trace("Updating {} takes {} ms", type_name, time);
    else if(time <= 50)
      spdlog::info("Updating {} takes {} ms", type_name, time);
    else
      spdlog::warn("Updating {} takes {} ms", type_name, time);

    std::free((void*)type_name);
  }
}

void Voxy::on_render()
{
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  for(auto& system : m_systems)
    system->on_render(*this, m_world);
}

int main()
{
  Voxy voxy;
  voxy.run();
}

