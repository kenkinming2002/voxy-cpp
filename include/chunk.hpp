#ifndef CHUNK_HPP
#define CHUNK_HPP

#include <glm/glm.hpp>
#include <vector>
#include <memory>

#include <mesh.hpp>

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
public:
  std::vector<Layer> layers;
  Mesh               layers_mesh;
  bool               layers_need_remash;

public:
  Chunk(glm::ivec2 cpos);
};

#endif // CHUNK_HPP
