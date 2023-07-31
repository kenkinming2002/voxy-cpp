#ifndef WORLD_HPP
#define WORLD_HPP

#include <camera.hpp>
#include <dimension.hpp>
#include <text_renderer.hpp>

#include <SDL.h>

#include <system/chunk_generator.hpp>
#include <system/chunk_mesher.hpp>
#include <system/light.hpp>

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

public:
  void load(glm::ivec2 chunk_index);
  void load(glm::ivec2 center, int radius);

private:
  Camera    m_camera;
  Entity    m_player;
  Dimension m_dimension;

private:
  TextRenderer m_text_renderer;

private:
  std::unique_ptr<ChunkGeneratorSystem> m_chunk_generator_system;
  std::unique_ptr<ChunkMesherSystem>    m_chunk_mesher_system;
  std::unique_ptr<LightSystem>          m_light_system;
};

#endif // WORLD_HPP
