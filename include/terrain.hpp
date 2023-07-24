#ifndef TERRAIN_HPP
#define TERRAIN_HPP

#include <camera.hpp>
#include <lights.hpp>
#include <chunk.hpp>

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <unordered_map>

struct Terrain
{
  gl::Program                           program;
  std::unordered_map<glm::ivec2, Chunk> chunks;

  Terrain();

  void load(glm::vec2 center, float radius);
  void unload(glm::vec2 center, float radius);

  void update(float dt);
  void render(const Camera& camera, const Lights& lights);
};

#endif // TERRAIN_HPP
