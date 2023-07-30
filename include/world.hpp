#ifndef WORLD_HPP
#define WORLD_HPP

#include <camera.hpp>
#include <chunk_manager.hpp>
#include <text_renderer.hpp>

#include <SDL.h>

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

private:
  Camera       m_camera;
  Entity       m_player;
  ChunkManager m_chunk_manager;

private:
  TextRenderer m_text_renderer;
};

#endif // WORLD_HPP
