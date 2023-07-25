#include <blocks.hpp>

#include <glm/gtx/hash.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/io.hpp>

#include <perlin.hpp>

#include <random>
#include <optional>
#include <iostream>

constexpr static float CAVE_WORM_RATIO = 0.2f;
constexpr static int   CAVE_WORM_SEGMENTS = 50;
constexpr static float CAVE_WORM_SEGMENT_LENGTH = 5.0f;

template <class T>
inline void hash_combine(std::size_t& seed, const T& v)
{
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

/*******************************
 * Coordinate System Utilities *
 *******************************/
static glm::ivec3 global_to_local(glm::ivec3 position, glm::ivec2 chunk_position)
{
  return glm::ivec3(
      position.x - chunk_position.x * Blocks::WIDTH,
      position.y - chunk_position.y * Blocks::WIDTH,
      position.z
  );
}

static glm::vec3 global_to_local(glm::vec3 position, glm::ivec2 chunk_position)
{
  return glm::vec3(
      position.x - chunk_position.x * Blocks::WIDTH,
      position.y - chunk_position.y * Blocks::WIDTH,
      position.z
  );
}

static glm::ivec3 local_to_global(glm::ivec3 position, glm::ivec2 chunk_position)
{
  return glm::ivec3(
      position.x + chunk_position.x * Blocks::WIDTH,
      position.y + chunk_position.y * Blocks::WIDTH,
      position.z
  );
}

static glm::vec3 local_to_global(glm::vec3 position, glm::ivec2 chunk_position)
{
  return glm::vec3(
      position.x + chunk_position.x * Blocks::WIDTH,
      position.y + chunk_position.y * Blocks::WIDTH,
      position.z
  );
}

struct Worm
{
  struct Node
  {
    glm::vec3 center;
    float     radius;
  };
  std::vector<Node> nodes;
};

static std::optional<Worm> generate_worm(size_t seed, glm::ivec2 chunk_position)
{
  hash_combine(seed, chunk_position);
  std::mt19937 prng(seed);

  size_t perlin_seed_x      = prng();
  size_t perlin_seed_y      = prng();
  size_t perlin_seed_z      = prng();
  size_t perlin_seed_radius = prng();

  Worm worm;
  if(std::uniform_real_distribution<float>(0.0f, 1.0f)(prng) > CAVE_WORM_RATIO)
    return std::nullopt;

  glm::vec3 chunk_anchor;
  chunk_anchor.x = chunk_position.x * Blocks::WIDTH;
  chunk_anchor.y = chunk_position.y * Blocks::WIDTH;
  chunk_anchor.z = 0.0f;

  glm::vec3 local_origin;
  local_origin.x = std::uniform_real_distribution<float>(0, Blocks::WIDTH-1)(prng);
  local_origin.y = std::uniform_real_distribution<float>(0, Blocks::WIDTH-1)(prng);
  local_origin.z = std::uniform_real_distribution<float>(0, 30.0)           (prng);

  glm::vec3 origin = chunk_anchor + local_origin;

  glm::vec3 position = origin;
  for(unsigned i=0; i<CAVE_WORM_SEGMENTS; ++i)
  {
    // 1: Record the node
    Worm::Node node;
    node.center = position;
    node.radius = 2.0f + perlin(perlin_seed_radius, position, 0.1f, 1.0f, 2.0f, 0.5f, 4);
    worm.nodes.push_back(node);

    // 2: Advance the worm
    //    TODO: implement and use overloads for perlin that output in range [-a, a]
    glm::vec3 direction;
    direction.x = perlin(perlin_seed_x, position, 0.1f, 1.0f, 2.0f, 0.5f, 4) - 1.0f;
    direction.y = perlin(perlin_seed_y, position, 0.1f, 1.0f, 2.0f, 0.5f, 4) - 1.0f;
    direction.z = perlin(perlin_seed_z, position, 0.1f, 1.0f, 2.0f, 0.5f, 4) - 1.0f;
    if(glm::length2(direction) < 1e-4)
      direction = glm::vec3(0.0f, 0.0f, 1.0f);

    position += CAVE_WORM_SEGMENT_LENGTH * glm::normalize(direction);
  }

  return worm;
}

Blocks::Blocks(size_t world_seed, glm::ivec2 cpos)
{
  std::mt19937 world_prng(world_seed);
  size_t world_stone_seed = world_prng();
  size_t world_grass_seed = world_prng();

  // 1: Terrain generation
  // TODO: More flexible terrain layer system
  int stone_heights[Blocks::WIDTH][Blocks::WIDTH];
  for(int cy=0; cy<Blocks::WIDTH; ++cy)
    for(int cx=0; cx<Blocks::WIDTH; ++cx)
    {
      glm::vec2 pos = glm::vec2(Blocks::WIDTH * cpos) + glm::vec2(cx, cy);
      stone_heights[cy][cx] = perlin(world_stone_seed, pos, 0.03f, 40.0f, 2.0f, 0.5f, 4);
    }

  int grass_heights[Blocks::WIDTH][Blocks::WIDTH];
  for(int cy=0; cy<Blocks::WIDTH; ++cy)
    for(int cx=0; cx<Blocks::WIDTH; ++cx)
    {
      glm::vec2 pos = glm::vec2(Blocks::WIDTH * cpos) + glm::vec2(cx, cy);
      grass_heights[cy][cx] = perlin(world_grass_seed, pos, 0.01f, 5.0f, 2.0f, 0.5f, 2);
    }

  int max_height = 0;
  for(int cy=0; cy<Blocks::WIDTH; ++cy)
    for(int cx=0; cx<Blocks::WIDTH; ++cx)
      if(max_height < stone_heights[cy][cx]+grass_heights[cy][cx])
        max_height = stone_heights[cy][cx]+grass_heights[cy][cx];

  for(int cz=0; cz<max_height; ++cz)
  {
    Blocks::Slice slice;
    for(int cy=0; cy<Blocks::WIDTH; ++cy)
      for(int cx=0; cx<Blocks::WIDTH; ++cx)
        if(cz <= stone_heights[cy][cx])
          slice.blocks[cy][cx] = Block {
            .presence = true,
            .color    = glm::vec3(0.7, 0.7, 0.7),
          };
        else if(cz <= stone_heights[cy][cx] + grass_heights[cy][cx])
          slice.blocks[cy][cx] = Block {
            .presence = true,
            .color    = glm::vec3(0.2, 1.0, 0.2),
          };
        else
          slice.blocks[cy][cx] = Block {
            .presence = false,
          };

    m_slices.push_back(slice);
  }

  // 2: Cave generation
  int radius = CAVE_WORM_SEGMENTS * CAVE_WORM_SEGMENT_LENGTH / WIDTH;
  glm::ivec2 corner1 = cpos - glm::ivec2(radius, radius);
  glm::ivec2 corner2 = cpos + glm::ivec2(radius, radius);
  for(int y = corner1.y; y <= corner2.y; ++y)
    for(int x = corner1.x; x <= corner2.x; ++x)
    {
      // TODO: Worms caching
      glm::ivec2 neighbour_chunk_position = glm::ivec2(x, y);
      std::optional<Worm> worm = generate_worm(world_seed, neighbour_chunk_position);
      if(worm)
        for(auto node : worm->nodes)
          explode(global_to_local(node.center, cpos), node.radius);
    }
}

Block Blocks::get(glm::ivec3 position) const
{
  if(position.x < 0 || position.x >= width())  return Block{ .presence = false };
  if(position.y < 0 || position.y >= width())  return Block{ .presence = false };
  if(position.z < 0 || position.z >= height()) return Block{ .presence = false };
  return m_slices[position.z].blocks[position.y][position.x];
}

void Blocks::set(glm::ivec3 position, Block block)
{
  if(position.x < 0 || position.x >= width())  return;
  if(position.y < 0 || position.y >= width())  return;
  if(position.z < 0 || position.z >= height()) return;
  m_slices[position.z].blocks[position.y][position.x] = block;
}

void Blocks::explode(glm::vec3 center, float radius)
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
          set(pos, Block{ .presence = false });
      }
}

