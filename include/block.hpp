#ifndef BLOCK_HPP
#define BLOCK_HPP

#include <glm/glm.hpp>

#include <stdint.h>

struct Block
{
  static constexpr uint32_t ID_STONE = 0;
  static constexpr uint32_t ID_GRASS = 1;

  static const Block NONE;
  static const Block STONE;
  static const Block GRASS;

  uint32_t presence : 1;
  uint32_t id       : 31;
};

inline const Block Block::NONE  = { .presence = 0, };
inline const Block Block::STONE = { .presence = 1, .id = Block::ID_STONE, };
inline const Block Block::GRASS = { .presence = 1, .id = Block::ID_GRASS, };

struct BlockData
{
  uint32_t texture_indices[6];
};

#endif // BLOCK_HPP
