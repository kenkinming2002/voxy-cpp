#ifndef WORLD_HPP
#define WORLD_HPP

#include <mesh.hpp>
#include <camera.hpp>
#include <timer.hpp>
#include <chunk.hpp>

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <unordered_map>
#include <utility>

#include <stddef.h>
#include <stdint.h>

struct Light
{
  glm::vec3 pos;
  glm::vec3 ambient;
  glm::vec3 diffuse;
};

struct World
{
  Camera camera;

  gl::Program light_program;
  gl::Program chunk_program;

  Light light;
  Mesh  light_mesh;

  std::unordered_map<glm::ivec2, Chunk> chunks;

  // Chunk generation
  World();

  void unload(glm::vec2 center, float radius);
  void load(glm::vec2 center, float radius);

  void handle_event(SDL_Event event);
  void update(float dt);

  void render();
};

#endif // WORLD_HPP
