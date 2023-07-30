#ifndef CHUNK_HPP
#define CHUNK_HPP

#include <chunk_defs.hpp>

#include <block.hpp>
#include <mesh.hpp>

#include <glm/glm.hpp>

#include <memory>
#include <optional>
#include <vector>

struct ChunkData
{
  Block blocks[CHUNK_HEIGHT][CHUNK_WIDTH][CHUNK_WIDTH];
};

class ChunkGenerator;
class Dimension;
struct Chunk
{
public:
  Chunk();

public:
  void update(glm::ivec2 chunk_position, const Dimension& dimension, const std::vector<BlockData>& block_datas);

public:
  std::optional<Block> get_block(glm::ivec3 position) const;
  bool set_block(glm::ivec3 position, Block block);
  void explode(glm::vec3 center, float radius);

public:
  void generate(glm::ivec2 chunk_position, const ChunkGenerator& chunk_generator);
  void remash(glm::ivec2 chunk_position, const Dimension& dimension, const std::vector<BlockData>& block_datas);

  void major_invalidate_mesh();
  void minor_invalidate_mesh();

public:
  std::unique_ptr<ChunkData> data;
  std::unique_ptr<Mesh>      mesh;

  bool     mesh_invalidated_major = false;
  bool     mesh_invalidated_minor = false;
  uint32_t last_remash_tick;
};

#endif // CHUNK_HPP
