#ifndef SYSTEM_CHUNK_MESHER_HPP
#define SYSTEM_CHUNK_MESHER_HPP

#include <glm/fwd.hpp>
#include <memory>
#include <stddef.h>

class Dimension;
class ChunkMesherSystem
{
public:
  static std::unique_ptr<ChunkMesherSystem> create();

public:
  virtual void update_chunk(Dimension& dimension, glm::ivec2 chunk_index) = 0;
  virtual void remesh_chunk(Dimension& dimension, glm::ivec2 chunk_index) = 0;
  virtual ~ChunkMesherSystem() = default;
};

#endif // SYSTEM_CHUNK_MESHER_HPP

