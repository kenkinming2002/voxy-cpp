#include <application.hpp>

#include <world.hpp>
#include <world_config.hpp>
#include <camera.hpp>

#include <world_generator.hpp>
#include <world_renderer.hpp>

#include <player_controller.hpp>

#include <physics.hpp>
#include <light.hpp>

#include <graphics/wireframe_renderer.hpp>

#include <debug_renderer.hpp>

#include <spdlog/spdlog.h>

class Voxy : public Application
{
public:
  Voxy();

private:
  void on_key(int key, int scancode, int action, int mods) override;

private:
  void on_update(float dt) override;
  void on_render()         override;

private:
  WorldConfig m_world_config;
  World       m_world;

  Camera      m_camera;
  bool        m_third_person;

  std::unique_ptr<WorldGenerator> m_world_generator;
  std::unique_ptr<WorldRenderer>  m_world_renderer;

  std::unique_ptr<PlayerController> m_player_controller;

  std::unique_ptr<graphics::WireframeRenderer> m_wireframe_renderer;

  std::unique_ptr<DebugRenderer> m_debug_renderer;
};

Voxy::Voxy()
{
  m_world_config = load_world_config("world");

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
          .dimension = glm::vec3(0.9f, 0.9f, 1.9f),
          .eye          = 1.5f,
        },
        {
          .id = 0,
          .transform = {
            .position = glm::vec3(0.0f, 0.0f, 50.0f),
            .rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
          },
          .velocity     = glm::vec3(0.0f, 0.0f, 0.0f),
          .dimension = glm::vec3(0.9f, 0.9f, 1.9f),
          .eye          = 1.5f,
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
  m_third_person = true;

  m_world_generator   = std::make_unique<WorldGenerator>(m_world_config.generation);
  m_world_renderer    = std::make_unique<WorldRenderer>("world", m_world_config);

  m_player_controller = std::make_unique<PlayerController>();

  m_wireframe_renderer = std::make_unique<graphics::WireframeRenderer>();

  m_debug_renderer = std::make_unique<DebugRenderer>();
}

void Voxy::on_key(int key, int scancode, int action, int mods)
{
  if(key == GLFW_KEY_F5 && action == GLFW_PRESS)
    m_third_person = !m_third_person;
}

void Voxy::on_update(float dt)
{
  m_world_generator->update(m_world);
  m_player_controller->update(*this, m_world, dt);

  update_physics(m_world, dt);
  update_light(m_world);

  m_debug_renderer->update(dt);
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
  if(m_third_person)
    m_camera.transform.position -= player_entity.transform.local_forward() * 5.0f;

  m_world_renderer->render(m_camera, m_world, m_third_person, *m_wireframe_renderer);
  m_player_controller->render(m_camera, m_world, *m_wireframe_renderer);

  m_debug_renderer->render(*this, m_world);
}

int main()
{
  Voxy voxy;
  voxy.run();
}

