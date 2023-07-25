#ifndef TERRAIN_HPP
#define TERRAIN_HPP

#include <camera.hpp>
#include <lights.hpp>
#include <chunk.hpp>

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <unordered_set>
#include <unordered_map>

#include <variant>
#include <optional>

#include <mutex>
#include <condition_variable>

#include <thread>

class Terrain
{
public:
  Terrain(size_t seed);

public:
  void load(glm::vec2 center, float radius);

  void update(float dt);
  void render(const Camera& camera, const Lights& lights);

private:
  void loader(std::stop_token stoken);

private:
  size_t m_seed;

  gl::Program m_program;

  std::mutex                  m_mutex;
  std::condition_variable_any m_cv;

  struct Pending {};
  struct Loading {};
  std::unordered_map<glm::ivec2, std::variant<Pending, Loading, Chunk>> m_states;

  std::vector<std::jthread> m_workers;
};

#endif // TERRAIN_HPP
