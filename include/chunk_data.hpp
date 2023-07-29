#ifndef CHUNK_DATA_HPP
#define CHUNK_DATA_HPP

#include <chunk_defs.hpp>
#include <chunk_info.hpp>

#include <glm/glm.hpp>

#include <unordered_map>
#include <vector>

#include <stdint.h>

struct Block
{
  static constexpr uint32_t ID_NONE  = 0;
  static constexpr uint32_t ID_STONE = 1;
  static constexpr uint32_t ID_GRASS = 2;

  static const Block NONE;
  static const Block STONE;
  static const Block GRASS;

  uint32_t id       : 31;
  uint32_t presence : 1;
};

inline const Block Block::NONE  = { .id = Block::ID_NONE,  .presence = 0, };
inline const Block Block::STONE = { .id = Block::ID_STONE, .presence = 1, };
inline const Block Block::GRASS = { .id = Block::ID_GRASS, .presence = 1, };

struct ChunkData
{
  struct Slice { Block blocks[CHUNK_WIDTH][CHUNK_WIDTH]; };
  std::vector<Slice> slices;
};

Block get_block(const ChunkData& chunk_data, glm::ivec3 position);
void set_block(ChunkData& chunk_data, glm::ivec3 position, Block block);
void explode(ChunkData& chunk_data, glm::vec3 center, float radius);

ChunkData generate_chunk_data(glm::ivec2 chunk_position, const std::unordered_map<glm::ivec2, ChunkInfo>& chunk_infos);

#endif // CHUNK_DATA_HPP
