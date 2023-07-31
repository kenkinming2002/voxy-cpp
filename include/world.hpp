#ifndef WORLD_HPP
#define WORLD_HPP

#include <camera.hpp>
#include <dimension.hpp>
#include <text_renderer.hpp>

#include <SDL.h>

#include <system/chunk_generator.hpp>
#include <system/chunk_mesher.hpp>
#include <system/chunk_renderer.hpp>
#include <system/light.hpp>
#include <system/physics.hpp>
#include <system/debug.hpp>

/**********
 * Entity *
 **********/
struct Entity
{
  Transform transform;

  glm::vec3 velocity;
  glm::vec3 bounding_box;
};

/*********
 * World *
 *********/
struct World
{
public:
  World(std::size_t seed);

public:
  void handle_event(SDL_Event event);
  void update(float dt);
  void render();

public:
  Dimension& dimension() { return m_dimension; }
  const Dimension& dimension() const { return m_dimension; }

  auto& camera() { return m_camera; }
  const auto& camera() const { return m_camera; }

  auto& player() { return m_player; }
  const auto& player() const { return m_player; }

private:
  Camera    m_camera;
  Entity    m_player;
  Dimension m_dimension;

private:
  std::unique_ptr<ChunkGeneratorSystem> m_chunk_generator_system;
  std::unique_ptr<ChunkMesherSystem>    m_chunk_mesher_system;
  std::unique_ptr<ChunkRendererSystem>  m_chunk_renderer_system;
  std::unique_ptr<LightSystem>          m_light_system;
  std::unique_ptr<PhysicsSystem>        m_physics_system;
  std::unique_ptr<DebugSystem>          m_debug_system;
};

#endif // WORLD_HPP
