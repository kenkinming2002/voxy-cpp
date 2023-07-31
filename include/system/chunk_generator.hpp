#ifndef SYSTEM_CHUNK_GENERATOR_HPP
#define SYSTEM_CHUNK_GENERATOR_HPP

#include <glm/fwd.hpp>
#include <memory>
#include <stddef.h>

struct World;
class ChunkGeneratorSystem
{
public:
  static std::unique_ptr<ChunkGeneratorSystem> create(std::size_t seed);

public:
  virtual void update(World& world) = 0;
  virtual ~ChunkGeneratorSystem() = default;
};

#endif // SYSTEM_CHUNK_GENERATOR_HPP
