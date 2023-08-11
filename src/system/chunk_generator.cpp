#include <system/chunk_generator.hpp>

#include <coordinates.hpp>
#include <perlin.hpp>
#include <world.hpp>

#include <spdlog/spdlog.h>

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/norm.hpp>

#include <unordered_map>

#include <future>
#include <random>

template <class T>
static size_t hash_combine(std::size_t seed, const T& v)
{
  std::hash<T> hasher;
  return seed ^ (hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2));
}

class ChunkGeneratorSystem : public System
{
private:
  static constexpr int CHUNK_LOAD_RADIUS = 2;

  static constexpr int CAVE_WORM_MAX = 2;
  static constexpr int CAVE_WORM_SEGMENT_MAX = 10;

  static constexpr float CAVE_WORM_MIN_HEIGHT = 10.0;
  static constexpr float CAVE_WORM_MAX_HEIGHT = 30.0;

  static constexpr float CAVE_WORM_MIN_RADIUS = 2.0;
  static constexpr float CAVE_WORM_MAX_RADIUS = 5.0;

  static constexpr float CAVE_WORM_STEP = 5.0f;

private:
  struct HeightMap
  {
    float heights[Chunk::WIDTH][Chunk::WIDTH];
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

private:
  template<typename Prng>
  static HeightMap generate_height_map(Prng& prng, glm::ivec2 chunk_index, float frequency, float amplitude, float lacunarity, float presistence, unsigned count)
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

  template<typename Prng>
  static std::vector<Worm> generate_worms(Prng& prng, glm::ivec2 chunk_index)
  {
    size_t seed_x      = prng();
    size_t seed_y      = prng() ;
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

  template<typename Prng>
  static ChunkInfo generate_chunk_info(Prng& prng_global, Prng& prng_local, glm::ivec2 chunk_index)
  {
    HeightMap         stone_height_map = generate_height_map(prng_global, chunk_index, 0.03f, 40.0f, 2.0f, 0.5f, 4);
    HeightMap         grass_height_map = generate_height_map(prng_global, chunk_index, 0.01f, 5.0f,  2.0f, 0.5f, 2);
    std::vector<Worm> worms            = generate_worms     (prng_local, chunk_index);

    return ChunkInfo {
      .stone_height_map = std::move(stone_height_map),
      .grass_height_map = std::move(grass_height_map),
      .worms            = std::move(worms),
    };
  }

  const ChunkInfo *try_generate_chunk_info(std::size_t seed, glm::ivec2 chunk_index)
  {
    if(auto it = m_chunk_infos.find(chunk_index); it != m_chunk_infos.end())
      return &it->second;

    if(auto it = m_chunk_info_futures.find(chunk_index); it != m_chunk_info_futures.end())
    {
      if(it->second.wait_for(std::chrono::nanoseconds(0)) == std::future_status::ready)
      {
        ChunkInfo chunk_info = it->second.get();
        m_chunk_info_futures.erase(it);

        auto [it, success] = m_chunk_infos.emplace(chunk_index, std::move(chunk_info));
        assert(success);
        return &it->second;
      }
      else
        return nullptr;
    }

    std::future<ChunkInfo> chunk_info_future = std::async(std::launch::async, [=]() {
      std::mt19937 prng_global(seed);
      std::mt19937 prng_local(hash_combine(seed, chunk_index));
      return generate_chunk_info(prng_global, prng_local, chunk_index);
    });

    auto [it, success] = m_chunk_info_futures.emplace(chunk_index, std::move(chunk_info_future));
    assert(success);
    return nullptr;
  }

  bool prepare_generate_chunk(std::size_t seed, glm::ivec2 chunk_index)
  {
    bool success = true;

    int        radius  = std::ceil(CAVE_WORM_SEGMENT_MAX * CAVE_WORM_STEP / Chunk::WIDTH);
    glm::ivec2 corner1 = chunk_index - glm::ivec2(radius, radius);
    glm::ivec2 corner2 = chunk_index + glm::ivec2(radius, radius);
    for(int y = corner1.y; y <= corner2.y; ++y)
      for(int x = corner1.x; x <= corner2.x; ++x)
        if(glm::ivec2 chunk_index = glm::ivec2(x, y); !try_generate_chunk_info(seed, chunk_index))
          success = false;

    return success;
  }

  Chunk do_generate_chunk(std::size_t seed, glm::ivec2 chunk_index)
  {
    Chunk chunk = {};

    const ChunkInfo& chunk_info = m_chunk_infos.at(chunk_index);

    // 1: Create terrain based on height maps
    for(int lz=0; lz<Chunk::HEIGHT; ++lz)
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
              block->id          = Block::ID_STONE;
              block->light_level = 0;
              block->sky         = false;
            }
            else if(lz < height1 + height2)
            {
              block->id          = Block::ID_GRASS;
              block->light_level = 0;
              block->sky         = false;
            }
            else
            {
              block->id          = Block::ID_NONE;
              block->light_level = 15;
              block->sky         = true;
            }
          }
        }

    // 2: Carve out caves based off worms
    int        radius  = std::ceil(CAVE_WORM_SEGMENT_MAX * CAVE_WORM_STEP / Chunk::WIDTH);
    glm::ivec2 corner1 = chunk_index - glm::ivec2(radius, radius);
    glm::ivec2 corner2 = chunk_index + glm::ivec2(radius, radius);
    for(int y = corner1.y; y <= corner2.y; ++y)
      for(int x = corner1.x; x <= corner2.x; ++x)
      {
        const ChunkInfo& neighbour_chunk_info  = m_chunk_infos.at(glm::ivec2(x, y));
        for(const Worm& worm : neighbour_chunk_info.worms)
          for(const Worm::Node& node : worm.nodes)
            chunk.explode(global_to_local(node.center, chunk_index), node.radius);
      }

    chunk.mesh_invalidated_major = true;
    chunk.mesh_invalidated_minor = false;
    chunk.last_remash_tick       = SDL_GetTicks();
    return chunk;
  }

  void commit_generate_chunk(World& world, glm::ivec2 chunk_index)
  {
    for(int z=0; z<Chunk::HEIGHT; ++z)
      for(int y=0; y<Chunk::WIDTH; ++y)
        for(int x=0; x<Chunk::WIDTH; ++x)
          world.invalidate_light(local_to_global(glm::ivec3(x, y, z), chunk_index));
  }

  void load(World& world, glm::ivec2 chunk_index)
  {
    if(!world.dimension.chunks.contains(chunk_index))
      if(prepare_generate_chunk(world.seed, chunk_index))
      {
        Chunk chunk = do_generate_chunk(world.seed, chunk_index);

        auto [_, success] = world.dimension.chunks.emplace(chunk_index, chunk);
        assert(success);

        commit_generate_chunk(world, chunk_index);

        spdlog::info("End generating chunk data at {}, {}", chunk_index.x, chunk_index.y);
      }
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

  void on_update(World& world, float dt) override
  {
    glm::ivec2 center = {
      std::floor(world.player.transform.position.x / Chunk::WIDTH),
      std::floor(world.player.transform.position.y / Chunk::WIDTH),
    };
    load(world, center, CHUNK_LOAD_RADIUS);
  }

private:
  std::unordered_map<glm::ivec2, ChunkInfo>              m_chunk_infos;
  std::unordered_map<glm::ivec2, std::future<ChunkInfo>> m_chunk_info_futures;
};

std::unique_ptr<System> create_chunk_generator_system()
{
  return std::make_unique<ChunkGeneratorSystem>();
}

