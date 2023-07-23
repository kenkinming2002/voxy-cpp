#ifndef WORLD_HPP
#define WORLD_HPP

#include <mesh.hpp>
#include <camera.hpp>
#include <timer.hpp>

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
  glm::vec3 specular;
};

struct Material
{
  glm::vec3 ambient;
  glm::vec3 diffuse;
  glm::vec3 specular;
  float     shininess;
};

struct Block
{
  bool      presence;
  glm::vec3 color;
};

struct Layer
{
  static constexpr int WIDTH = 16;
  Block blocks[WIDTH][WIDTH];
};

struct Chunk
{
  std::vector<Layer> layers;
};

struct World
{
  gl::Program light_program;
  gl::Program chunk_program;

  Light light;
  Mesh  light_mesh;

  Material                              chunk_material;
  std::unordered_map<glm::ivec2, Chunk> chunks;
  std::unordered_map<glm::ivec2, Mesh>  chunk_meshes;

  Camera camera;

  // Chunk generation
  World();

  void generate_chunk(glm::ivec2 cpos);
  void generate_chunk_mesh(glm::ivec2 cpos);

  void unload(glm::vec2 center, float radius);
  void load(glm::vec2 center, float radius);

  void handle_event(SDL_Event event);
  void update(float dt);

  void render();
};

#endif // WORLD_HPP
