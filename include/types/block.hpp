#ifndef BLOCK_HPP
#define BLOCK_HPP

#include <glm/glm.hpp>

#include <stdint.h>

struct Block
{
  static constexpr std::uint32_t ID_STONE = 0;
  static constexpr std::uint32_t ID_GRASS = 1;

  static const Block NONE;
  static const Block STONE;
  static const Block GRASS;

  std::uint32_t presence    : 1;
  std::uint32_t id          : 26;
  std::uint32_t sky         : 1;
  std::uint32_t light_level : 4;
};

inline const Block Block::NONE  = { .presence = 0, };
inline const Block Block::STONE = { .presence = 1, .id = Block::ID_STONE, };
inline const Block Block::GRASS = { .presence = 1, .id = Block::ID_GRASS, };

struct BlockData
{
  std::uint32_t texture_indices[6];
};

#endif // BLOCK_HPP
