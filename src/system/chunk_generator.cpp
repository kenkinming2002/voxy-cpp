#include <system/chunk_generator.hpp>

#include <coordinates.hpp>
#include <perlin.hpp>
#include <world.hpp>

#include <GLFW/glfw3.h>

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
  static constexpr int CHUNK_LOAD_RADIUS = 8;

private:
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
    std::vector<HeightMap> height_maps;
    std::vector<Worm>      worms;
  };

private:
  template<typename Prng>
  static std::vector<HeightMap> generate_height_maps(Prng& prng, const TerrainGenerationConfig& config, glm::ivec2 chunk_index)
  {
    std::vector<HeightMap> height_maps;
    for(const TerrainLayerGenerationConfig& terrain_layer_config : config.layers)
    {
      size_t seed = prng();

      HeightMap height_map;
      for(int y=0; y<CHUNK_WIDTH; ++y)
        for(int x=0; x<CHUNK_WIDTH; ++x)
          height_map.heights[y][x] = std::max(terrain_layer_config.base + perlin(seed, coordinates::local_to_global(glm::vec2(x, y), chunk_index),
              terrain_layer_config.frequency,
              terrain_layer_config.amplitude,
              terrain_layer_config.lacunarity,
              terrain_layer_config.persistence,
              terrain_layer_config.octaves
          ), 0.0f);
      height_maps.push_back(height_map);
    }
    return height_maps;
  }

  template<typename Prng>
  static std::vector<Worm> generate_worms(Prng& prng, const CavesGenerationConfig& config, glm::ivec2 chunk_index)
  {
    size_t seed_x      = prng();
    size_t seed_y      = prng() ;
    size_t seed_z      = prng();
    size_t seed_radius = prng();

    std::vector<Worm> worms;

    int worm_count = std::uniform_int_distribution<int>(0, config.max_per_chunk)(prng);
    for(int i=0; i<worm_count; ++i)
    {
      Worm worm;

      glm::vec3 local_origin;
      local_origin.x = std::uniform_real_distribution<float>(0, CHUNK_WIDTH-1)(prng);
      local_origin.y = std::uniform_real_distribution<float>(0, CHUNK_WIDTH-1)(prng);
      local_origin.z = std::uniform_real_distribution<float>(config.min_height, config.max_height)(prng);
      glm::vec3 origin = coordinates::local_to_global(local_origin, chunk_index);

      glm::vec3 position = origin;
      for(unsigned i=0; i<config.max_segment; ++i)
      {
        // 1: Record the node
        Worm::Node node;
        node.center = position;
        node.radius = config.radius + perlin(seed_radius, position,
          config.radius_frequency,
          config.radius_amplitude,
          config.radius_lacunarity,
          config.radius_persistence,
          config.radius_octaves
        );
        worm.nodes.push_back(node);

        // 2: Advance the worm
        glm::vec3 direction;
        direction.x = perlin(seed_x, position, config.dig_frequency, config.dig_amplitude, config.dig_lacunarity, config.dig_frequency, config.dig_octaves);
        direction.y = perlin(seed_y, position, config.dig_frequency, config.dig_amplitude, config.dig_lacunarity, config.dig_frequency, config.dig_octaves);
        direction.z = perlin(seed_z, position, config.dig_frequency, config.dig_amplitude, config.dig_lacunarity, config.dig_frequency, config.dig_octaves);
        if(glm::length2(direction) < 1e-4)
          direction = glm::vec3(0.0f, 0.0f, 1.0f);

        position += config.step * glm::normalize(direction);
      }

      worms.push_back(std::move(worm));
    }

    return worms;
  }

  template<typename Prng>
  static ChunkInfo generate_chunk_info(Prng& prng_global, Prng& prng_local, const WorldGenerationConfig& config, glm::ivec2 chunk_index)
  {
    std::vector<HeightMap> height_maps = generate_height_maps(prng_global, config.terrain, chunk_index);
    std::vector<Worm>      worms       = generate_worms(prng_local, config.caves, chunk_index);

    return ChunkInfo {
      .height_maps = std::move(height_maps),
      .worms       = std::move(worms),
    };
  }

  const ChunkInfo *try_generate_chunk_info(const WorldGenerationConfig& config, glm::ivec2 chunk_index)
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
      std::mt19937 prng_global(config.seed);
      std::mt19937 prng_local(hash_combine(config.seed, chunk_index));
      return generate_chunk_info(prng_global, prng_local, config, chunk_index);
    });

    auto [it, success] = m_chunk_info_futures.emplace(chunk_index, std::move(chunk_info_future));
    assert(success);
    return nullptr;
  }

  bool prepare_generate_chunk(const WorldGenerationConfig& config, glm::ivec2 chunk_index)
  {
    bool success = true;

    int        radius  = std::ceil(config.caves.max_segment * config.caves.step / CHUNK_WIDTH);
    glm::ivec2 corner1 = chunk_index - glm::ivec2(radius, radius);
    glm::ivec2 corner2 = chunk_index + glm::ivec2(radius, radius);
    for(int y = corner1.y; y <= corner2.y; ++y)
      for(int x = corner1.x; x <= corner2.x; ++x)
        if(glm::ivec2 chunk_index = glm::ivec2(x, y); !try_generate_chunk_info(config, chunk_index))
          success = false;

    return success;
  }

  ChunkData do_generate_chunk(const WorldGenerationConfig& config, glm::ivec2 chunk_index)
  {
    ChunkData chunk = {};

    const ChunkInfo& chunk_info = m_chunk_infos.at(chunk_index);

    // 1: Create terrain based on height maps
    for(int z=0; z<CHUNK_HEIGHT; ++z)
      for(int y=0; y<CHUNK_WIDTH; ++y)
        for(int x=0; x<CHUNK_WIDTH; ++x)
        {
          Block *block = chunk.get_block(glm::ivec3(x, y, z));
          if(!block) [[unlikely]]
            continue;

          float height = 0.0f;
          for(size_t i=0; i<chunk_info.height_maps.size(); ++i)
          {
            const HeightMap& height_map = chunk_info.height_maps[i];

            height += height_map.heights[y][x];
            if(z < height)
            {
              block->id          = config.terrain.layers[i].block_id;
              block->light_level = 0;
              block->sky         = false;
              goto done;
            }
          }

          block->id          = Block::ID_NONE;
          block->light_level = 15;
          block->sky         = true;
done:;
        }

    // 2: Carve out caves based off worms
    glm::ivec2 center  = chunk_index;
    int        radius  = std::ceil(config.caves.max_segment * config.caves.step / CHUNK_WIDTH);
    glm::ivec2 corner1 = center - glm::ivec2(radius, radius);
    glm::ivec2 corner2 = center + glm::ivec2(radius, radius);
    for(int y = corner1.y; y <= corner2.y; ++y)
      for(int x = corner1.x; x <= corner2.x; ++x)
      {
        const ChunkInfo& neighbour_chunk_info  = m_chunk_infos.at(glm::ivec2(x, y));
        for(const Worm& worm : neighbour_chunk_info.worms)
          for(const Worm::Node& node : worm.nodes)
          {
            glm::vec3  center  = coordinates::global_to_local(node.center, chunk_index);
            float      radius  = node.radius;
            glm::ivec3 corner1 = glm::floor(glm::vec3(center) - glm::vec3(radius, radius, radius));
            glm::ivec3 corner2 = glm::ceil (glm::vec3(center) + glm::vec3(radius, radius, radius));
            for(int z = corner1.z; z<=corner2.z; ++z)
              for(int y = corner1.y; y<=corner2.y; ++y)
                for(int x = corner1.x; x<=corner2.x; ++x)
                {
                  glm::ivec3 position(x, y, z);
                  if(glm::length2(glm::vec3(position) - center) < radius * radius)
                    if(Block* block = chunk.get_block(position))
                    {
                      block->id          = Block::ID_NONE;
                      block->light_level = 0;
                      block->sky         = false;
                      chunk.pending_lighting_updates.insert(position);
                    }
                }
          }
      }

    chunk.mesh_invalidated_major = true;
    chunk.mesh_invalidated_minor = false;
    chunk.last_remash_time       = glfwGetTime();

    return chunk;
  }

  void load(const WorldConfig& world_config, WorldData& world_data, glm::ivec2 chunk_index)
  {
    if(!world_data.dimension.chunks.contains(chunk_index))
      if(prepare_generate_chunk(world_config.generation, chunk_index))
      {
        ChunkData chunk = do_generate_chunk(world_config.generation, chunk_index);

        auto [_, success] = world_data.dimension.chunks.emplace(chunk_index, std::move(chunk));
        assert(success);

        spdlog::info("End generating chunk data at {}, {}", chunk_index.x, chunk_index.y);
      }
  }

  void load(const WorldConfig& world_config, WorldData& world_data, glm::ivec2 center, int radius)
  {
    for(int cy = center.y - radius; cy <= center.y + radius; ++cy)
      for(int cx = center.x - radius; cx <= center.x + radius; ++cx)
      {
        glm::ivec2 chunk_index(cx, cy);
        load(world_config, world_data, chunk_index);
      }
  }

  void on_update(Application& application, const WorldConfig& world_config, WorldData& world_data, float dt) override
  {
    glm::ivec2 center = {
      std::floor(world_data.player.transform.position.x / CHUNK_WIDTH),
      std::floor(world_data.player.transform.position.y / CHUNK_WIDTH),
    };
    load(world_config, world_data, center, CHUNK_LOAD_RADIUS);
  }

private:
  std::unordered_map<glm::ivec2, ChunkInfo>              m_chunk_infos;
  std::unordered_map<glm::ivec2, std::future<ChunkInfo>> m_chunk_info_futures;
};

std::unique_ptr<System> create_chunk_generator_system()
{
  return std::make_unique<ChunkGeneratorSystem>();
}

