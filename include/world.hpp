#ifndef WORLD_HPP
#define WORLD_HPP

#include <mesh.hpp>
#include <camera.hpp>

#include <glm/glm.hpp>

#include <unordered_map>
#include <utility>

#include <stddef.h>
#include <stdint.h>

template<>
struct std::hash<glm::ivec2>
{
  std::size_t operator()(const glm::ivec2& v) const noexcept
  {
    std::size_t hash = 0b0111011101010101011011101111011001000011101010110101001111110010;
    hash ^= v.x *      0b1010111011010110101010110101010101101010101010101010010101010101;
    hash ^= v.y *      0b1010111010101110101010101101010101010001010111010101101101010100;
    hash = (hash * 17) ^ (hash * 31) ^ (hash * 71);
    hash = hash ^ (hash << 21);
    return hash;
  }
};


template<>
struct std::hash<glm::ivec3>
{
  std::size_t operator()(const glm::ivec3& v) const noexcept
  {
    std::size_t hash = 0b1001010111011010110110101010101110100101010011010101001010101001;
    hash ^= v.x *      0b1010010010101010101101110010101111010110011000101001010100101111;
    hash ^= v.y *      0b0010101001101011110101011101110101100110100110010001110101011110;
    hash ^= v.z *      0b1110101010110011101110001101100111010101010111010000101110110100;
    return hash;
  }
};

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
