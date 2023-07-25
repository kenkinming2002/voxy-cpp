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
  int height() const { return m_slices.size(); }

public:
  Block get(glm::ivec3 position) const;
  void set(glm::ivec3 position, Block block);
  void explode(glm::vec3 center, float radius);

private:
  struct Slice { Block blocks[WIDTH][WIDTH]; };
  std::vector<Slice> m_slices;
};

#endif // BLOCKS_HPP
