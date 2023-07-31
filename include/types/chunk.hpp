#ifndef CHUNK_HPP
#define CHUNK_HPP

#include <types/block.hpp>

#include <mesh.hpp>

#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

#include <memory>
#include <optional>
#include <vector>

static constexpr int CHUNK_WIDTH  = 16;
static constexpr int CHUNK_HEIGHT = 256;

struct ChunkData
{
  Block blocks[CHUNK_HEIGHT][CHUNK_WIDTH][CHUNK_WIDTH];
};

struct Chunk
{
public:
  std::optional<Block> get_block(glm::ivec3 position) const
  {
    if(!data)                                        return std::nullopt;
    if(position.x < 0 || position.x >= CHUNK_WIDTH)  return std::nullopt;
    if(position.y < 0 || position.y >= CHUNK_WIDTH)  return std::nullopt;
    if(position.z < 0 || position.z >= CHUNK_HEIGHT) return std::nullopt;

    return data->blocks[position.z][position.y][position.x];
  }

  bool set_block(glm::ivec3 position, Block block)
  {
    if(!data)                                        return false;
    if(position.x < 0 || position.x >= CHUNK_WIDTH)  return false;
    if(position.y < 0 || position.y >= CHUNK_WIDTH)  return false;
    if(position.z < 0 || position.z >= CHUNK_HEIGHT) return false;

    data->blocks[position.z][position.y][position.x] = block;
    return true;
  }

  void explode(glm::vec3 center, float radius)
  {
    // TODO: Culling
    glm::ivec3 corner1 = glm::floor(center - glm::vec3(radius, radius, radius));
    glm::ivec3 corner2 = glm::ceil (center + glm::vec3(radius, radius, radius));
    for(int z = corner1.z; z<=corner2.z; ++z)
      for(int y = corner1.y; y<=corner2.y; ++y)
        for(int x = corner1.x; x<=corner2.x; ++x)
        {
          glm::ivec3 pos = { x, y, z };
          if(glm::length2(glm::vec3(pos) - center) < radius * radius)
            set_block(pos, Block{ .presence = false });
        }
  }

public:
  void major_invalidate_mesh() { mesh_invalidated_major = true; }
  void minor_invalidate_mesh() { mesh_invalidated_minor = true; }

public:
  std::unique_ptr<ChunkData> data;
  std::unique_ptr<Mesh>      mesh;

  bool     mesh_invalidated_major;
  bool     mesh_invalidated_minor;
  uint32_t last_remash_tick;
};

#endif // CHUNK_HPP
