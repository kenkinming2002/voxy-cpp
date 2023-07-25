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

struct Blocks
{
  static constexpr int WIDTH = 16;

  constexpr int width()  const { return WIDTH; }
  constexpr int height() const { return layers.size(); }

  Block get(glm::ivec3 lpos) const
  {
    if(lpos.x < 0 || lpos.x >= width())  return Block{ .presence = false };
    if(lpos.y < 0 || lpos.y >= width())  return Block{ .presence = false };
    if(lpos.y < 0 || lpos.z >= height()) return Block{ .presence = false };
    return layers[lpos.z].blocks[lpos.y][lpos.x];
  }

  void set(glm::ivec3 lpos, Block block)
  {
    if(lpos.x < 0 || lpos.x >= width())  return;
    if(lpos.y < 0 || lpos.y >= width())  return;
    if(lpos.y < 0 || lpos.z >= height()) return;
    layers[lpos.z].blocks[lpos.y][lpos.x] = block;
  }

  struct Layer { Block blocks[WIDTH][WIDTH]; };
  std::vector<Layer> layers;
};


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
