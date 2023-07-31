#ifndef DIMENSION_HPP
#define DIMENSION_HPP

#include <chunk.hpp>

#include <texture_array.hpp>

#include <gl.hpp>

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <unordered_map>
#include <unordered_set>

struct Camera;
struct Dimension
{
public:
  std::optional<Block> get_block(glm::ivec3 position) const;
  bool set_block(glm::ivec3 position, Block block);

public:
  void major_invalidate_mesh(glm::ivec3 position);
  void minor_invalidate_mesh(glm::ivec3 position);

public:
  void lighting_invalidate(glm::ivec3 position);

public:
  TextureArray           blocks_texture_array;
  std::vector<BlockData> block_datas;

public:
  std::unordered_map<glm::ivec2, Chunk> chunks;
  std::unordered_set<glm::ivec3>        pending_lighting_updates;
};

#endif // DIMENSION_HPP
