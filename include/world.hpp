#ifndef WORLD_HPP
#define WORLD_HPP

#include <camera.hpp>

#include <graphics/texture_array.hpp>
#include <graphics/mesh.hpp>

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <unordered_map>
#include <unordered_set>

#include <future>

#include <optional>
#include <memory>

struct Entity
{
  Transform transform;

  glm::vec3 velocity;
  glm::vec3 bounding_box;
  bool      collided;
};

struct BlockData
{
  std::uint32_t texture_indices[6];
};

struct Block
{
  static constexpr std::uint32_t ID_STONE = 0;
  static constexpr std::uint32_t ID_GRASS = 1;
  static constexpr std::uint32_t ID_NONE  = 2;

  static const Block NONE;
  static const Block STONE;
  static const Block GRASS;

  std::uint32_t id          : 27;
  std::uint32_t sky         : 1;
  std::uint32_t light_level : 4;
};

inline const Block Block::STONE = Block{ .id = ID_STONE };
inline const Block Block::GRASS = Block{ .id = ID_GRASS };
inline const Block Block::NONE  = Block{ .id = ID_NONE };

struct Chunk
{
public:
  static constexpr int WIDTH  = 16;
  static constexpr int HEIGHT = 256;

public:
  Block blocks[Chunk::HEIGHT][Chunk::WIDTH][Chunk::WIDTH];

  bool     mesh_invalidated_major;
  bool     mesh_invalidated_minor;
  uint32_t last_remash_tick;

public:
  Block* get_block(glm::ivec3 position);
  const Block* get_block(glm::ivec3 position) const;
  void explode(glm::vec3 center, float radius);

  void major_invalidate_mesh();
  void minor_invalidate_mesh();
};

struct Dimension
{
public:
  graphics::TextureArray blocks_texture_array;
  std::vector<BlockData> block_datas;

  std::unordered_map<glm::ivec2, Chunk> chunks;
  std::unordered_set<glm::ivec3> pending_lighting_updates;

public:
  Block* get_block(glm::ivec3 position);
  const Block* get_block(glm::ivec3 position) const;

  void major_invalidate_mesh(glm::ivec3 position);
  void minor_invalidate_mesh(glm::ivec3 position);

  void lighting_invalidate(glm::ivec3 position);
};

struct World
{
public:
  std::size_t seed;

  Camera    camera;
  Entity    player;
  Dimension dimension;

  std::optional<glm::ivec3> placement;
  std::optional<glm::ivec3> selection;

public:
  Block* get_block(glm::ivec3 position);
  const Block* get_block(glm::ivec3 position) const;

  void invalidate_mesh_major(glm::ivec3 position);
  void invalidate_mesh_minor(glm::ivec3 position);

  void invalidate_light(glm::ivec3 position);
};


/***********
 * Helpers *
 ***********/
static constexpr glm::ivec3 DIRECTIONS[] = {
  {-1, 0, 0},
  { 1, 0, 0},
  {0, -1, 0},
  {0,  1, 0},
  {0, 0, -1},
  {0, 0,  1},
};

template<glm::length_t L, typename T, glm::qualifier Q>
inline glm::vec<L, T, Q> global_to_local(glm::vec<L, T, Q> position, glm::ivec2 chunk_index)
{
  position[0] -= chunk_index[0] * Chunk::WIDTH;
  position[1] -= chunk_index[1] * Chunk::WIDTH;
  return position;
}

template<glm::length_t L, typename T, glm::qualifier Q>
inline glm::vec<L, T, Q> local_to_global(glm::vec<L, T, Q> position, glm::ivec2 chunk_index)
{
  position[0] += chunk_index[0] * Chunk::WIDTH;
  position[1] += chunk_index[1] * Chunk::WIDTH;
  return position;
}

#endif // WORLD_HPP

