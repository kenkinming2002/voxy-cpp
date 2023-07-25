#ifndef CHUNK_HPP
#define CHUNK_HPP

#include <glm/glm.hpp>
#include <vector>
#include <memory>

#include <mesh.hpp>
#include <blocks.hpp>

struct Chunk
{
public:
  Blocks blocks;
  Mesh   blocks_mesh;
  bool   blocks_need_remash;

public:
  Chunk(glm::ivec2 cpos);
};

#endif // CHUNK_HPP
