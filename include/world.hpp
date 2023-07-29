#ifndef WORLD_HPP
#define WORLD_HPP

#include <chunk_defs.hpp>
#include <chunk_info.hpp>
#include <chunk_data.hpp>
#include <chunk_manager.hpp>

#include <camera.hpp>
#include <light.hpp>
#include <mesh.hpp>
#include <transform.hpp>

#include <gl.hpp>

#include <SDL.h>

#include <glm/glm.hpp>

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
  Camera             m_camera;
  std::vector<Light> m_lights;
  Entity             m_player;
  ChunkManager       m_chunk_manager;
};

#endif // WORLD_HPP
