#ifndef CHUNK_MESHER_HPP
#define CHUNK_MESHER_HPP

#include <glm/fwd.hpp>
#include <memory>
#include <stddef.h>

class Dimension;
class ChunkMesher
{
public:
  static std::unique_ptr<ChunkMesher> create();

public:
  virtual void update_chunk(Dimension& dimension, glm::ivec2 chunk_index) = 0;
  virtual void remesh_chunk(Dimension& dimension, glm::ivec2 chunk_index) = 0;
  virtual ~ChunkMesher() = default;
};

#endif // CHUNK_MESHER_HPP

