#include <application.hpp>

#include <world.hpp>
#include <world_config.hpp>
#include <camera.hpp>

#include <world_generator.hpp>
#include <world_renderer.hpp>

#include <player_controller.hpp>

#include <physics.hpp>
#include <light.hpp>

#include <debug_renderer.hpp>

#include <spdlog/spdlog.h>

class Voxy : public Application
{
public:
  Voxy();

private:
  void on_update(float dt) override;
  void on_render()         override;

private:
  WorldConfig m_world_config;
  World       m_world;
  Camera      m_camera;

  std::unique_ptr<WorldGenerator> m_world_generator;
  std::unique_ptr<WorldRenderer>  m_world_renderer;

  std::unique_ptr<PlayerController> m_player_controller;

  std::unique_ptr<DebugRenderer> m_debug_renderer;
};

Voxy::Voxy()
{
  // TODO: Load config from files
  m_world_config = {
    .generation = {
      .seed = 0b1011011010110101110110110101110101011010110101011111010100011010,
      .terrain = {
        .layers = {
          { .block_id = BLOCK_ID_STONE, .base = 40.0f, .frequency = 0.03f, .amplitude = 20.0f, .lacunarity = 2.0f, .persistence = 0.5f, .octaves = 4, },
          { .block_id = BLOCK_ID_GRASS, .base = 5.0f,  .frequency = 0.01f, .amplitude = 2.0f,  .lacunarity = 2.0f, .persistence = 0.5f, .octaves = 2, },
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
    .blocks = {
      {
        .textures = {
          "assets/stone.png",
          "assets/stone.png",
          "assets/stone.png",
          "assets/stone.png",
          "assets/stone.png",
          "assets/stone.png",
        }
      }, {
        .textures = {
          "assets/grass_side.png",
          "assets/grass_side.png",
          "assets/grass_side.png",
          "assets/grass_side.png",
          "assets/grass_bottom.png",
          "assets/grass_top.png",
        },
      }
    },
    .entities = {
      {
        .model   = "assets/character/idk.obj",
        .texture = "assets/character/idk.png",
      },
    },
  };

  m_world = {
    .dimension = {
      .entities = {
        {
          .id = 0,
          .transform = {
            .position = glm::vec3(0.0f, 0.0f, 50.0f),
            .rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
          },
          .velocity     = glm::vec3(0.0f, 0.0f, 0.0f),
          .bounding_box = glm::vec3(0.9f, 0.9f, 1.9f),
          .eye          = 0.5f,
        },
        {
          .id = 0,
          .transform = {
            .position = glm::vec3(0.0f, 0.0f, 50.0f),
            .rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
          },
          .velocity     = glm::vec3(0.0f, 0.0f, 0.0f),
          .bounding_box = glm::vec3(0.9f, 0.9f, 1.9f),
          .eye          = 0.5f,
        },
      }
    },
    .player = {
      .entity_id = 0,
      .placement = std::nullopt,
      .selection = std::nullopt,
    },
  };

  m_camera = {
    .aspect = 1024.0f / 720.0f,
    .fovy   = 45.0f,
  };

  m_world_generator   = std::make_unique<WorldGenerator>(m_world_config.generation);
  m_world_renderer    = std::make_unique<WorldRenderer>(m_world_config);
  m_player_controller = std::make_unique<PlayerController>();
  m_debug_renderer    = std::make_unique<DebugRenderer>();
}

void Voxy::on_update(float dt)
{
  m_world_generator->update(m_world);
  m_player_controller->update(*this, m_world, dt);

  update_physics(m_world, dt);
  update_light(m_world);
}

void Voxy::on_render()
{
  int width, height;
  glfw_get_framebuffer_size(width, height);

  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glViewport(0, 0, width, height);

  const Entity& player_entity = m_world.dimension.entities.at(m_world.player.entity_id);

  m_camera.transform             = player_entity.transform;
  m_camera.transform.position.z += player_entity.eye;
  m_camera.aspect = (float)width / (float)height;

  m_world_renderer->render(m_camera, m_world);
  m_player_controller->render(m_camera, m_world);

  m_debug_renderer->render(*this, m_world);
}

int main()
{
  Voxy voxy;
  voxy.run();
}

