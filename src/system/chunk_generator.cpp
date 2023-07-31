#include <system/chunk_generator.hpp>

#include <world.hpp>
#include <chunk_coords.hpp>
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

struct HeightMap
{
  float heights[CHUNK_WIDTH][CHUNK_WIDTH];
};

struct Worm
{
  struct Node
  {
    glm::vec3 center;
    float     radius;
  };
  std::vector<Node> nodes;
};

struct ChunkInfo
{
  HeightMap         stone_height_map;
  HeightMap         grass_height_map;
  std::vector<Worm> worms;
};

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
  for(int ly=0; ly<CHUNK_WIDTH; ++ly)
    for(int lx=0; lx<CHUNK_WIDTH; ++lx)
    {
      glm::ivec2 position = local_to_global(glm::ivec2(lx, ly), chunk_index);
      height_map.heights[ly][lx] = perlin(seed, position, frequency, amplitude, lacunarity, presistence, count);
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
    local_origin.x = std::uniform_real_distribution<float>(0, CHUNK_WIDTH-1)(prng);
    local_origin.y = std::uniform_real_distribution<float>(0, CHUNK_WIDTH-1)(prng);
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


class ChunkGeneratorSystemImpl : public ChunkGeneratorSystem
{
public:
  ChunkGeneratorSystemImpl(std::size_t seed)
    : m_seed(seed)
  {
    unsigned count = std::thread::hardware_concurrency();
    for(unsigned i=0; i<count; ++i)
      m_workers.emplace_back(std::bind(&ChunkGeneratorSystemImpl::work, this, std::placeholders::_1));
  }

private:
  void update(World& world) override
  {
    glm::ivec2 center = {
      std::floor(world.player().transform.position.x / CHUNK_WIDTH),
      std::floor(world.player().transform.position.y / CHUNK_WIDTH),
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
    if(!world.dimension().chunks[chunk_index].data)
    {
      if(!try_generate_chunk(world, chunk_index))
        return;

      for(int lz=0; lz<CHUNK_HEIGHT; ++lz)
        for(int ly=0; ly<CHUNK_WIDTH; ++ly)
          for(int lx=0; lx<CHUNK_WIDTH; ++lx)
          {
            glm::ivec3 position = { lx, ly, lz };
            world.dimension().lighting_invalidate(local_to_global(position, chunk_index));
          }
    }
  }

  bool try_generate_chunk(World& world, glm::ivec2 chunk_index)
  {
    Chunk& chunk = world.dimension().chunks[chunk_index];
    if(chunk.data)
    {
      spdlog::warn("Chunk at {}, {} has already been generated", chunk_index.x, chunk_index.y);
      return false;
    }

    if(!can_generate_chunk(chunk_index))
      return false;

    chunk.data = std::make_unique<ChunkData>();

    const ChunkInfo* chunk_info = try_get_chunk_info(chunk_index);
    assert(chunk_info);

    // 1: Create terrain based on height maps
    int max_height = 0;
    for(int ly=0; ly<CHUNK_WIDTH; ++ly)
      for(int lx=0; lx<CHUNK_WIDTH; ++lx)
      {
        int total_height = chunk_info->stone_height_map.heights[ly][lx]
          + chunk_info->grass_height_map.heights[ly][lx];
        max_height = std::max(max_height, total_height);
      }

    for(int lz=0; lz<max_height; ++lz)
    {
      for(int ly=0; ly<CHUNK_WIDTH; ++ly)
        for(int lx=0; lx<CHUNK_WIDTH; ++lx)
        {
          int height1 = chunk_info->stone_height_map.heights[ly][lx];
          int height2 = chunk_info->stone_height_map.heights[ly][lx] + chunk_info->grass_height_map.heights[ly][lx];
          chunk.set_block(glm::ivec3(lx, ly, lz), lz < height1 ? Block::STONE :
                                                  lz < height2 ? Block::GRASS :
                                                  Block::NONE);
        }
    }

    // 2: Carve out caves based off worms
    int        radius  = std::ceil(CAVE_WORM_SEGMENT_MAX * CAVE_WORM_STEP / CHUNK_WIDTH);
    glm::ivec2 corner1 = chunk_index - glm::ivec2(radius, radius);
    glm::ivec2 corner2 = chunk_index + glm::ivec2(radius, radius);
    for(int cy = corner1.y; cy <= corner2.y; ++cy)
      for(int cx = corner1.x; cx <= corner2.x; ++cx)
      {
        glm::ivec2       neighbour_chunk_index = glm::ivec2(cx, cy);
        const ChunkInfo *neighbour_chunk_info     = try_get_chunk_info(neighbour_chunk_index);
        assert(neighbour_chunk_info);

        for(const Worm& worm : neighbour_chunk_info->worms)
          for(const Worm::Node& node : worm.nodes)
            chunk.explode(global_to_local(node.center, chunk_index), node.radius);
      }

    return true;
  }

private:
  bool can_generate_chunk(glm::ivec2 chunk_index) const
  {
    int        radius  = std::ceil(CAVE_WORM_SEGMENT_MAX * CAVE_WORM_STEP / CHUNK_WIDTH);
    glm::ivec2 corner1 = chunk_index - glm::ivec2(radius, radius);
    glm::ivec2 corner2 = chunk_index + glm::ivec2(radius, radius);
    for(int cy = corner1.y; cy <= corner2.y; ++cy)
      for(int cx = corner1.x; cx <= corner2.x; ++cx)
      {
        glm::ivec2 neighbour_chunk_index = glm::ivec2(cx, cy);
        if(!try_get_chunk_info(neighbour_chunk_index))
          return false;
      }

    return true;
  }

  const ChunkInfo *try_get_chunk_info(glm::ivec2 chunk_index) const
  {
    {
      std::lock_guard guard(m_mutex);
      if(auto it = m_chunk_infos.find(chunk_index); it != m_chunk_infos.end())
        return &it->second;

      if(m_pending_chunk_infos.contains(chunk_index)) return nullptr;
      if(m_loading_chunk_infos.contains(chunk_index)) return nullptr;
      m_pending_chunk_infos.insert(chunk_index);
    }
    m_cv.notify_one();
    return nullptr;
  }

  void work(std::stop_token stoken)
  {
    std::unique_lock lk(m_mutex);
    for(;;)
    {
      m_cv.wait(lk, stoken, [this]() { return !m_pending_chunk_infos.empty(); });
      if(stoken.stop_requested())
        return;

      while(!m_pending_chunk_infos.empty())
      {
        glm::ivec2 chunk_index = *m_pending_chunk_infos.begin();
        m_pending_chunk_infos.erase(m_pending_chunk_infos.begin());
        m_loading_chunk_infos.insert(chunk_index);

        lk.unlock();

        spdlog::info("Begin Generating chunk info at {}, {}", chunk_index.x, chunk_index.y);
        ChunkInfo chunk_info = generate_chunk_info(chunk_index, m_seed);
        spdlog::info("End Generating chunk info at {}, {}", chunk_index.x, chunk_index.y);

        lk.lock();

        m_loading_chunk_infos.erase(chunk_index);
        m_chunk_infos.emplace(chunk_index, std::move(chunk_info));
        if(stoken.stop_requested())
          return;
      }
    }
  }

private:
  std::size_t m_seed;

private:
  mutable std::shared_mutex           m_mutex;
  mutable std::condition_variable_any m_cv;

  mutable std::unordered_set<glm::ivec2>    m_pending_chunk_infos;
  mutable std::unordered_set<glm::ivec2>    m_loading_chunk_infos;
  std::unordered_map<glm::ivec2, ChunkInfo> m_chunk_infos;

private:
  std::vector<std::jthread> m_workers;
};

std::unique_ptr<ChunkGeneratorSystem> ChunkGeneratorSystem::create(std::size_t seed)
{
  return std::make_unique<ChunkGeneratorSystemImpl>(seed);
}

