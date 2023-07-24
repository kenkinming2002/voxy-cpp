#ifndef TERRAIN_HPP
#define TERRAIN_HPP

#include <camera.hpp>
#include <lights.hpp>
#include <chunk.hpp>

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <unordered_map>

#include <mutex>
#include <condition_variable>

#include <thread>

struct Terrain
{
  gl::Program program;

  std::mutex                            chunks_mutex;
  std::unordered_map<glm::ivec2, Chunk> chunks;

  std::condition_variable_any load_cv;
  std::mutex                  load_mutex;
  glm::vec2                   center;
  float                       radius;

  std::jthread worker;

  Terrain();

  void load(glm::vec2 center, float radius);

  void update(float dt);
  void render(const Camera& camera, const Lights& lights);
};

#endif // TERRAIN_HPP
