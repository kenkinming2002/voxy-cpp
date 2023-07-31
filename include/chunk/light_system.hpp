#ifndef CHUNK_LIGHT_SYSTEM_HPP
#define CHUNK_LIGHT_SYSTEM_HPP

#include <glm/fwd.hpp>
#include <memory>
#include <stddef.h>

class Dimension;
class ChunkLightSystem
{
public:
  static std::unique_ptr<ChunkLightSystem> create();

public:
  virtual void update(Dimension& dimension) = 0;
  virtual ~ChunkLightSystem() = default;
};

#endif // CHUNK_LIGHT_SYSTEM_HPP
