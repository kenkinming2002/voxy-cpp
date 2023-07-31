#ifndef SYSTEM_CHUNK_GENERATOR_HPP
#define SYSTEM_CHUNK_GENERATOR_HPP

#include <glm/fwd.hpp>
#include <memory>
#include <stddef.h>

class Dimension;
class ChunkGeneratorSystem
{
public:
  static std::unique_ptr<ChunkGeneratorSystem> create(std::size_t seed);

public:
  virtual bool try_generate_chunk(Dimension& dimension, glm::ivec2 chunk_index) = 0;
  virtual ~ChunkGeneratorSystem() = default;
};

#endif // SYSTEM_CHUNK_GENERATOR_HPP
