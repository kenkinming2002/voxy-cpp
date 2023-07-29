#ifndef CHUNK_DATA_HPP
#define CHUNK_DATA_HPP

#include <chunk_defs.hpp>
#include <block.hpp>

#include <glm/glm.hpp>

#include <vector>

class ChunkManager;
struct ChunkData
{
  struct Slice { Block blocks[CHUNK_WIDTH][CHUNK_WIDTH]; };
  std::vector<Slice> slices;

  Block get_block(glm::ivec3 position) const;
  void set_block(glm::ivec3 position, Block block);
  void explode(glm::vec3 center, float radius);

  static ChunkData generate(glm::ivec2 chunk_position, const ChunkManager& chunk_manager);
};

#endif // CHUNK_DATA_HPP
