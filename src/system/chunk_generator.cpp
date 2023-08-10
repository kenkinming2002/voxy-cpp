#include <system/chunk_generator.hpp>

#include <world.hpp>

#include <perlin.hpp>

#include <spdlog/spdlog.h>

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/norm.hpp>

#include <unordered_set>
#include <unordered_map>

#include <condition_variable>
#include <shared_mutex>
#include <thread>

#include <random>

static constexpr int CHUNK_LOAD_RADIUS = 2;

static constexpr int CAVE_WORM_MAX = 2;
static constexpr int CAVE_WORM_SEGMENT_MAX = 10;

static constexpr float CAVE_WORM_MIN_HEIGHT = 10.0;
static constexpr float CAVE_WORM_MAX_HEIGHT = 30.0;

static constexpr float CAVE_WORM_MIN_RADIUS = 2.0;
static constexpr float CAVE_WORM_MAX_RADIUS = 5.0;

static constexpr float CAVE_WORM_STEP = 5.0f;

template <class T>
static size_t hash_combine(std::size_t seed, const T& v)
{
    std::hash<T> hasher;
    return seed ^ (hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2));
}

static HeightMap generate_height_map(glm::ivec2 chunk_index, std::mt19937& prng, float frequency, float amplitude, float lacunarity, float presistence, unsigned count)
{
  size_t seed = prng();

  HeightMap height_map;
  for(int ly=0; ly<Chunk::WIDTH; ++ly)
    for(int lx=0; lx<Chunk::WIDTH; ++lx)
    {
      glm::ivec2 position = local_to_global(glm::ivec2(lx, ly), chunk_index);
      height_map.heights[ly][lx] = perlin(seed, glm::vec2(position), frequency, amplitude, lacunarity, presistence, count);
    }

  return height_map;
}

static std::vector<Worm> generate_worms(glm::ivec2 chunk_index, std::mt19937& prng)
{
  size_t seed_x      = prng();
  size_t seed_y      = prng();
  size_t seed_z      = prng();
  size_t seed_radius = prng();

  std::vector<Worm> worms;

  int worm_count = std::uniform_int_distribution<int>(0, CAVE_WORM_MAX)(prng);
  for(int i=0; i<worm_count; ++i)
  {
    Worm worm;

    glm::vec3 local_origin;
    local_origin.x = std::uniform_real_distribution<float>(0, Chunk::WIDTH-1)(prng);
    local_origin.y = std::uniform_real_distribution<float>(0, Chunk::WIDTH-1)(prng);
    local_origin.z = std::uniform_real_distribution<float>(CAVE_WORM_MIN_HEIGHT, CAVE_WORM_MAX_HEIGHT)(prng);
    glm::vec3 origin = local_to_global(local_origin, chunk_index);

    glm::vec3 position = origin;
    for(unsigned i=0; i<CAVE_WORM_SEGMENT_MAX; ++i)
    {
      // TODO: Consider implementing overloads of perlin noise that allow specifying min/max

      // 1: Record the node
      Worm::Node node;
      node.center = position;
      node.radius = CAVE_WORM_MIN_RADIUS + perlin(seed_radius, position, 0.1f, CAVE_WORM_MAX_RADIUS-CAVE_WORM_MIN_RADIUS, 2.0f, 0.5f, 1);
      worm.nodes.push_back(node);

      // 2: Advance the worm
      glm::vec3 direction;
      direction.x = perlin(seed_x, position, 0.1f, 1.0f, 2.0f, 0.5f, 4) - 1.0f;
      direction.y = perlin(seed_y, position, 0.1f, 1.0f, 2.0f, 0.5f, 4) - 1.0f;
      direction.z = perlin(seed_z, position, 0.1f, 1.0f, 2.0f, 0.5f, 4) - 1.0f;
      if(glm::length2(direction) < 1e-4)
        direction = glm::vec3(0.0f, 0.0f, 1.0f);

      position += CAVE_WORM_STEP * glm::normalize(direction);
    }

    worms.push_back(std::move(worm));
  }

  return worms;
}

static ChunkInfo generate_chunk_info(glm::ivec2 chunk_index, size_t seed)
{
  std::mt19937 prng_global(seed);
  std::mt19937 prng_local(hash_combine(seed, chunk_index));

  HeightMap         stone_height_map = generate_height_map(chunk_index, prng_global, 0.03f, 40.0f, 2.0f, 0.5f, 4);
  HeightMap         grass_height_map = generate_height_map(chunk_index, prng_global, 0.01f, 5.0f,  2.0f, 0.5f, 2);
  std::vector<Worm> worms            = generate_worms(chunk_index, prng_local);

  return ChunkInfo {
    .stone_height_map = std::move(stone_height_map),
    .grass_height_map = std::move(grass_height_map),
    .worms            = std::move(worms),
  };
}


class ChunkGeneratorSystem : public System
{
private:
  void on_update(World& world, float dt) override
  {
    // 1: Retrieve
    for(auto it = world.dimension.chunk_info_futures.begin(); it != world.dimension.chunk_info_futures.end(); )
      if(it->second.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready)
      {
        spdlog::info("Finish generating chunk info at {}, {}", it->first.x, it->first.y);
        world.dimension.chunks[it->first].info = std::make_unique<ChunkInfo>(it->second.get());
        world.dimension.chunk_info_futures.erase(it++);
      }
      else
        ++it;

    // 2: Loading
    glm::ivec2 center = {
      std::floor(world.player.transform.position.x / Chunk::WIDTH),
      std::floor(world.player.transform.position.y / Chunk::WIDTH),
    };
    load(world, center, CHUNK_LOAD_RADIUS);
  }

  void load(World& world, glm::ivec2 center, int radius)
  {
    for(int cy = center.y - radius; cy <= center.y + radius; ++cy)
      for(int cx = center.x - radius; cx <= center.x + radius; ++cx)
      {
        glm::ivec2 chunk_index(cx, cy);
        load(world, chunk_index);
      }
  }

  void load(World& world, glm::ivec2 chunk_index)
  {
    Chunk& chunk = world.dimension.chunks[chunk_index];
    if(!chunk.data)
      if(can_generate_chunk(world, chunk_index))
      {
        generate_chunk(world, chunk_index);
        for(int lz=0; lz<Chunk::HEIGHT; ++lz)
          for(int ly=0; ly<Chunk::WIDTH; ++ly)
            for(int lx=0; lx<Chunk::WIDTH; ++lx)
            {
              glm::ivec3 position = { lx, ly, lz };
              world.invalidate_light(local_to_global(position, chunk_index));
            }
      }
  }

  bool can_generate_chunk(World& world, glm::ivec2 chunk_index) const
  {
    int        radius  = std::ceil(CAVE_WORM_SEGMENT_MAX * CAVE_WORM_STEP / Chunk::WIDTH);
    glm::ivec2 corner1 = chunk_index - glm::ivec2(radius, radius);
    glm::ivec2 corner2 = chunk_index + glm::ivec2(radius, radius);
    for(int cy = corner1.y; cy <= corner2.y; ++cy)
      for(int cx = corner1.x; cx <= corner2.x; ++cx)
      {
        glm::ivec2 neighbour_chunk_index = glm::ivec2(cx, cy);
        Chunk&     neighbour_chunk       = world.dimension.chunks[neighbour_chunk_index];
        if(!neighbour_chunk.info)
        {
          if(!world.dimension.chunk_info_futures.contains(neighbour_chunk_index))
            world.dimension.chunk_info_futures.emplace(neighbour_chunk_index, std::async(std::launch::async, [=, seed=world.seed]() { return generate_chunk_info(neighbour_chunk_index, seed); }));

          return false;
        }
      }

    return true;
  }

  void generate_chunk(World& world, glm::ivec2 chunk_index)
  {
    spdlog::info("Begin generating chunk data at {}, {}", chunk_index.x, chunk_index.y);

    Chunk& chunk = world.dimension.chunks[chunk_index];
    chunk.data = std::make_unique<ChunkData>();

    const ChunkInfo& chunk_info = *world.dimension.chunks[chunk_index].info.get();

    // 1: Create terrain based on height maps
    int max_height = 0;
    for(int ly=0; ly<Chunk::WIDTH; ++ly)
      for(int lx=0; lx<Chunk::WIDTH; ++lx)
      {
        int total_height = chunk_info.stone_height_map.heights[ly][lx]
                         + chunk_info.grass_height_map.heights[ly][lx];
        max_height = std::max(max_height, total_height);
      }

    for(int lz=0; lz<max_height; ++lz)
      for(int ly=0; ly<Chunk::WIDTH; ++ly)
        for(int lx=0; lx<Chunk::WIDTH; ++lx)
        {
          int height1 = chunk_info.stone_height_map.heights[ly][lx];
          int height2 = chunk_info.grass_height_map.heights[ly][lx];
          Block *block = chunk.get_block(glm::ivec3(lx, ly, lz));
          if(block) [[likely]]
          {
            if(lz < height1)
            {
              block->presence    = true;
              block->id          = Block::ID_STONE;
              block->light_level = 0;
              block->sky         = false;
            }
            else if(lz < height1 + height2)
            {
              block->presence    = true;
              block->id          = Block::ID_GRASS;
              block->light_level = 0;
              block->sky         = false;
            }
            else
            {
              block->presence    = false;
              block->light_level = 15;
              block->sky         = true;
            }
          }
        }

    // 2: Carve out caves based off worms
    int        radius  = std::ceil(CAVE_WORM_SEGMENT_MAX * CAVE_WORM_STEP / Chunk::WIDTH);
    glm::ivec2 corner1 = chunk_index - glm::ivec2(radius, radius);
    glm::ivec2 corner2 = chunk_index + glm::ivec2(radius, radius);
    for(int cy = corner1.y; cy <= corner2.y; ++cy)
      for(int cx = corner1.x; cx <= corner2.x; ++cx)
      {
        glm::ivec2       neighbour_chunk_index = glm::ivec2(cx, cy);
        const ChunkInfo& neighbour_chunk_info  = *world.dimension.chunks[neighbour_chunk_index].info.get();

        for(const Worm& worm : neighbour_chunk_info.worms)
          for(const Worm::Node& node : worm.nodes)
            chunk.explode(global_to_local(node.center, chunk_index), node.radius);
      }

    spdlog::info("End generating chunk data at {}, {}", chunk_index.x, chunk_index.y);
  }
};

std::unique_ptr<System> create_chunk_generator_system()
{
  return std::make_unique<ChunkGeneratorSystem>();
}

