#ifndef WORLD_HPP
#define WORLD_HPP

#include <camera.hpp>

#include <texture_array.hpp>
#include <mesh.hpp>

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <unordered_map>
#include <unordered_set>
#include <optional>
#include <memory>

static constexpr int CHUNK_WIDTH  = 16;
static constexpr int CHUNK_HEIGHT = 256;

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

  static const Block NONE;
  static const Block STONE;
  static const Block GRASS;

  std::uint32_t presence    : 1;
  std::uint32_t id          : 26;
  std::uint32_t sky         : 1;
  std::uint32_t light_level : 4;
};

inline const Block Block::NONE  = Block{ .presence = false };
inline const Block Block::STONE = Block{ .presence = true, .id = ID_STONE };
inline const Block Block::GRASS = Block{ .presence = true, .id = ID_GRASS };

struct ChunkData
{
  Block blocks[CHUNK_HEIGHT][CHUNK_WIDTH][CHUNK_WIDTH];
};

struct Chunk
{
public:
  std::unique_ptr<ChunkData> data;
  std::unique_ptr<Mesh>      mesh;

  bool     mesh_invalidated_major;
  bool     mesh_invalidated_minor;
  uint32_t last_remash_tick;

public:
  Block *get_block(glm::ivec3 position);
  const Block *get_block(glm::ivec3 position) const;
  void explode(glm::vec3 center, float radius);

  void major_invalidate_mesh();
  void minor_invalidate_mesh();
};

struct Dimension
{
public:
  TextureArray           blocks_texture_array;
  std::vector<BlockData> block_datas;

  std::unordered_map<glm::ivec2, Chunk> chunks;
  std::unordered_set<glm::ivec3>        pending_lighting_updates;

public:
  Block *get_block(glm::ivec3 position);
  const Block *get_block(glm::ivec3 position) const;

  void major_invalidate_mesh(glm::ivec3 position);
  void minor_invalidate_mesh(glm::ivec3 position);

  void lighting_invalidate(glm::ivec3 position);
};

struct World
{
  Camera    camera;
  Entity    player;
  Dimension dimension;

  std::optional<glm::ivec3> placement;
  std::optional<glm::ivec3> selection;
};


/*********
 * Chunk *
 *********/
inline Block* Chunk::get_block(glm::ivec3 position)
{
  if(!data)                                        return nullptr;
  if(position.x < 0 || position.x >= CHUNK_WIDTH)  return nullptr;
  if(position.y < 0 || position.y >= CHUNK_WIDTH)  return nullptr;
  if(position.z < 0 || position.z >= CHUNK_HEIGHT) return nullptr;

  return &data->blocks[position.z][position.y][position.x];
}

inline const Block* Chunk::get_block(glm::ivec3 position) const
{
  if(!data)                                        return nullptr;
  if(position.x < 0 || position.x >= CHUNK_WIDTH)  return nullptr;
  if(position.y < 0 || position.y >= CHUNK_WIDTH)  return nullptr;
  if(position.z < 0 || position.z >= CHUNK_HEIGHT) return nullptr;

  return &data->blocks[position.z][position.y][position.x];
}

inline void Chunk::explode(glm::vec3 center, float radius)
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
          if(Block *block = get_block(pos))
            block->presence = false;
      }
}

inline void Chunk::major_invalidate_mesh() { mesh_invalidated_major = true; }
inline void Chunk::minor_invalidate_mesh() { mesh_invalidated_minor = true; }

/*************
 * Dimension *
 *************/
inline int modulo(int a, int b)
{
  int value = a % b;
  if(value < 0)
    value += b;

  assert(0 <= value);
  assert(value < b);
  return value;
}

inline Block* Dimension::get_block(glm::ivec3 position)
{
  // FIXME: Refactor me`
  glm::ivec3 local_position = {
    modulo(position.x, CHUNK_WIDTH),
    modulo(position.y, CHUNK_WIDTH),
    position.z
  };

  glm::ivec2 chunk_index = {
    (position.x - local_position.x) / CHUNK_WIDTH,
    (position.y - local_position.y) / CHUNK_WIDTH,
  };

  auto it = chunks.find(chunk_index);
  if(it == chunks.end())
    return nullptr;

  return it->second.get_block(local_position);
}

inline const Block* Dimension::get_block(glm::ivec3 position) const
{
  // FIXME: Refactor me`
  glm::ivec3 local_position = {
    modulo(position.x, CHUNK_WIDTH),
    modulo(position.y, CHUNK_WIDTH),
    position.z
  };

  glm::ivec2 chunk_index = {
    (position.x - local_position.x) / CHUNK_WIDTH,
    (position.y - local_position.y) / CHUNK_WIDTH,
  };

  auto it = chunks.find(chunk_index);
  if(it == chunks.end())
    return nullptr;

  return it->second.get_block(local_position);
}

inline void Dimension::major_invalidate_mesh(glm::ivec3 position)
{
  glm::ivec3 local_position = {
    modulo(position.x, CHUNK_WIDTH),
    modulo(position.y, CHUNK_WIDTH),
    position.z
  };
  glm::ivec2 chunk_index = {
    (position.x - local_position.x) / CHUNK_WIDTH,
    (position.y - local_position.y) / CHUNK_WIDTH,
  };
  chunks[chunk_index].major_invalidate_mesh();
}

inline void Dimension::minor_invalidate_mesh(glm::ivec3 position)
{
  glm::ivec3 local_position = {
    modulo(position.x, CHUNK_WIDTH),
    modulo(position.y, CHUNK_WIDTH),
    position.z
  };
  glm::ivec2 chunk_index = {
    (position.x - local_position.x) / CHUNK_WIDTH,
    (position.y - local_position.y) / CHUNK_WIDTH,
  };
  chunks[chunk_index].minor_invalidate_mesh();
}

inline void Dimension::lighting_invalidate(glm::ivec3 position)
{
  pending_lighting_updates.insert(position);
}

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
  position[0] -= chunk_index[0] * CHUNK_WIDTH;
  position[1] -= chunk_index[1] * CHUNK_WIDTH;
  return position;
}

template<glm::length_t L, typename T, glm::qualifier Q>
inline glm::vec<L, T, Q> local_to_global(glm::vec<L, T, Q> position, glm::ivec2 chunk_index)
{
  position[0] += chunk_index[0] * CHUNK_WIDTH;
  position[1] += chunk_index[1] * CHUNK_WIDTH;
  return position;
}


#endif // WORLD_HPP

