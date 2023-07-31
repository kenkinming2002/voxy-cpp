#ifndef SYSTEM_CHUNK_MESHER_HPP
#define SYSTEM_CHUNK_MESHER_HPP

#include <glm/fwd.hpp>
#include <memory>
#include <stddef.h>

struct World;
class ChunkMesherSystem
{
public:
  static std::unique_ptr<ChunkMesherSystem> create();

public:
  virtual void update(World& world) = 0;
  virtual ~ChunkMesherSystem() = default;
};

#endif // SYSTEM_CHUNK_MESHER_HPP

