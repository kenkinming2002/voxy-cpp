#ifndef SYSTEM_CHUNK_RENDERER_HPP
#define SYSTEM_CHUNK_RENDERER_HPP

#include <glm/fwd.hpp>
#include <memory>
#include <stddef.h>

struct World;
class ChunkRendererSystem
{
public:
  static std::unique_ptr<ChunkRendererSystem> create();

public:
  virtual void render(const World& world) = 0;
  virtual ~ChunkRendererSystem() = default;
};

#endif // SYSTEM_CHUNK_RENDERER_HPP


