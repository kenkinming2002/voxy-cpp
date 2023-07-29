#ifndef CHUNK_DATA_HPP
#define CHUNK_DATA_HPP

#include <chunk_defs.hpp>
#include <block.hpp>

#include <glm/glm.hpp>

#include <optional>
#include <vector>

class ChunkManager;
struct ChunkData
{
public:
  struct Slice { Block blocks[CHUNK_WIDTH][CHUNK_WIDTH]; };
  std::vector<Slice> slices;

public:
  static ChunkData generate(glm::ivec2 chunk_position, const ChunkManager& chunk_manager);

public:
  std::optional<Block> get_block(glm::ivec3 position) const;
  bool set_block(glm::ivec3 position, Block block);

public:
  void explode(glm::vec3 center, float radius);

};

#endif // CHUNK_DATA_HPP
