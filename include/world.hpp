#ifndef WORLD_HPP
#define WORLD_HPP

#include <mesh.hpp>

#include <camera.hpp>
#include <lights.hpp>
#include <terrain.hpp>

#include <SDL.h>

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <unordered_map>
#include <utility>

#include <stddef.h>
#include <stdint.h>

struct World
{
  Camera camera;

  Lights  lights;
  Terrain terrain;

  World();

  void handle_event(SDL_Event event);

  void update(float dt);
  void render();
};

#endif // WORLD_HPP
