#pragma once

#include <cstdint>

struct Block
{
  static constexpr std::uint32_t ID_STONE = 0;
  static constexpr std::uint32_t ID_GRASS = 1;
  static constexpr std::uint32_t ID_NONE  = 2;

  std::uint32_t id            : 23;
  std::uint32_t sky           : 1;
  std::uint32_t light_level   : 4;
  std::uint32_t destroy_level : 4;
};

