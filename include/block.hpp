#pragma once

#include <cstdint>

struct Block
{
  static constexpr std::uint32_t ID_STONE = 0;
  static constexpr std::uint32_t ID_GRASS = 1;
  static constexpr std::uint32_t ID_NONE  = 2;

  static const Block NONE;
  static const Block STONE;
  static const Block GRASS;

  std::uint32_t id          : 27;
  std::uint32_t sky         : 1;
  std::uint32_t light_level : 4;
};

inline const Block Block::STONE = Block{ .id = ID_STONE };
inline const Block Block::GRASS = Block{ .id = ID_GRASS };
inline const Block Block::NONE  = Block{ .id = ID_NONE };

