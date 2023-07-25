#ifndef BLOCKS_HPP
#define BLOCKS_HPP

#include <glm/glm.hpp>
#include <vector>

struct Block
{
  bool      presence;
  glm::vec3 color;
};

class Blocks
{
public:
  static constexpr int WIDTH = 16;

public:
  Blocks(size_t seed, glm::ivec2 cpos);

public:
  int width()  const { return WIDTH; }
  int height() const { return m_layers.size(); }

public:
  Block get(glm::ivec3 lpos) const
  {
    if(lpos.x < 0 || lpos.x >= width())  return Block{ .presence = false };
    if(lpos.y < 0 || lpos.y >= width())  return Block{ .presence = false };
    if(lpos.z < 0 || lpos.z >= height()) return Block{ .presence = false };
    return m_layers[lpos.z].blocks[lpos.y][lpos.x];
  }

  void set(glm::ivec3 lpos, Block block)
  {
    if(lpos.x < 0 || lpos.x >= width())  return;
    if(lpos.y < 0 || lpos.y >= width())  return;
    if(lpos.z < 0 || lpos.z >= height()) return;
    m_layers[lpos.z].blocks[lpos.y][lpos.x] = block;
  }

private:
  struct Layer { Block blocks[WIDTH][WIDTH]; };
  std::vector<Layer> m_layers;
};

#endif // BLOCKS_HPP
