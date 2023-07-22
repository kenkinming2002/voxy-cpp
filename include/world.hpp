#ifndef WORLD_HPP
#define WORLD_HPP

#include <mesh.hpp>
#include <camera.hpp>

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <unordered_map>
#include <utility>

#include <stddef.h>
#include <stdint.h>

struct Chunk
{
  static constexpr int WIDTH = 16;

  typedef uint16_t Layer[WIDTH][WIDTH];
  uint16_t blocks[WIDTH][WIDTH][WIDTH];
};

struct World
{
  std::unordered_map<glm::ivec3, Chunk> chunks;
  std::unordered_map<glm::ivec3, Mesh>  chunk_meshes;

  void generate_chunk(glm::ivec3 cpos);
  void generate_chunk_mesh(glm::ivec3 cpos);

  void draw(const Camera& camera);
};

#endif // WORLD_HPP
