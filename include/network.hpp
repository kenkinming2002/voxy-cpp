#pragma once

#include <cstdint>
#include <world.hpp>

struct [[gnu::packed]] Message
{
  std::uint8_t  id;
  std::uint32_t size;
};

struct MessageChunk : Message
{
  glm::ivec2 position;
  Chunk      chunk;
};

struct MessageBlock : Message
{
  glm::ivec3 position;
  Block      block;
};

