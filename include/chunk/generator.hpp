#ifndef CHUNK_GENERATOR_HPP
#define CHUNK_GENERATOR_HPP

#include <glm/fwd.hpp>
#include <memory>
#include <stddef.h>

class Dimension;
class ChunkGenerator
{
public:
  static std::unique_ptr<ChunkGenerator> create(std::size_t seed);

public:
  virtual bool try_generate_chunk(Dimension& dimension, glm::ivec2 chunk_position) = 0;
  virtual ~ChunkGenerator() = default;
};

#endif // CHUNK_GENERATOR_HPP
