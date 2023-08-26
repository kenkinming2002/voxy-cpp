#include <world_generator.hpp>

#include <coordinates.hpp>
#include <noise.hpp>

#include <GLFW/glfw3.h>

#include <spdlog/spdlog.h>

#include <thread>
#include <random>

template<typename Prng>
static inline std::vector<HeightMap> generate_height_maps(Prng& prng, const TerrainGenerationConfig& config, glm::ivec2 chunk_index)
{
  std::vector<HeightMap> height_maps;
  for(const LayerGenerationConfig& terrain_layer_config : config.layers)
  {
    size_t seed = prng();

    HeightMap height_map;
    for(int y=0; y<CHUNK_WIDTH; ++y)
      for(int x=0; x<CHUNK_WIDTH; ++x)
        height_map.heights[y][x] = std::max(terrain_layer_config.height_base + noise(seed, coordinates::local_to_global(glm::vec2(x, y), chunk_index), terrain_layer_config.height_noise), 0.0f);
    height_maps.push_back(height_map);
  }
  return height_maps;
}

template<typename Prng>
static inline std::vector<Worm> generate_worms(Prng& prng, const CavesGenerationConfig& config, glm::ivec2 chunk_index)
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
      node.radius = config.radius_base + noise(seed_radius, position, config.radius_noise);
      worm.nodes.push_back(node);

      // 2: Advance the worm
      glm::vec3 direction;
      direction.x = noise(seed_x, position, config.dig_noise);
      direction.y = noise(seed_y, position, config.dig_noise);
      direction.z = noise(seed_z, position, config.dig_noise);
      if(glm::length2(direction) < 1e-4)
        direction = glm::vec3(0.0f, 0.0f, 1.0f);

      position += config.step * glm::normalize(direction);
    }

    worms.push_back(std::move(worm));
  }

  return worms;
}

template<typename Prng>
static inline ChunkInfo generate_chunk_info(Prng& prng_global, Prng& prng_local, const GenerationConfig& config, glm::ivec2 chunk_index)
{
  std::vector<HeightMap> height_maps = generate_height_maps(prng_global, config.terrain, chunk_index);
  std::vector<Worm>      worms       = generate_worms(prng_local, config.caves, chunk_index);
  return ChunkInfo {
    .height_maps = std::move(height_maps),
    .worms       = std::move(worms),
  };
}

template <class T>
static inline size_t hash_combine(std::size_t seed, const T& v)
{
  std::hash<T> hasher;
  return seed ^ (hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2));
}

WorldGenerator::WorldGenerator(GenerationConfig config) : m_config(std::move(config)) {}

void WorldGenerator::update(World& world, LightManager& light_manager)
{
  const Entity& player_entity = world.dimension.entities.at(world.player.entity_id);
  glm::ivec2 center = {
    std::floor(player_entity.transform.position.x / CHUNK_WIDTH),
    std::floor(player_entity.transform.position.y / CHUNK_WIDTH),
  };
  try_load(world, light_manager, center, CHUNK_LOAD_RADIUS);
}

void WorldGenerator::try_load(World& world, LightManager& light_manager, glm::ivec2 chunk_index, int radius)
{
  for(int dy = -radius; dy <= radius; ++dy)
    for(int dx = -radius; dx <= radius; ++dx)
      if(dx * dx + dy * dy <= radius * radius)
      {
        glm::ivec2 position = chunk_index + glm::ivec2(dx, dy);
        try_load(world, light_manager, position);
      }
}

void WorldGenerator::try_load(World& world, LightManager& light_manager, glm::ivec2 chunk_index)
{
  if(world.dimension.chunks.find(chunk_index) != world.dimension.chunks.end())
    return;

  // 0: Setup
  int radius = std::ceil(m_config.caves.max_segment * m_config.caves.step / CHUNK_WIDTH);

  // 1: Check if we can generate the chunk now
  bool can_load = true;
  for(int y = chunk_index.y - radius; y <= chunk_index.y + radius; ++y)
    for(int x = chunk_index.x - radius; x <= chunk_index.x + radius; ++x)
    {
      glm::ivec2 neighbour_chunk_index = glm::ivec2(x, y);
      auto it = m_chunk_infos.find(neighbour_chunk_index);
      if(it == m_chunk_infos.end())
      {
        bool success;
        std::tie(it, success) = m_chunk_infos.emplace(neighbour_chunk_index, [this, neighbour_chunk_index]() {
          std::mt19937 prng_global(m_config.seed);
          std::mt19937 prng_local(hash_combine(m_config.seed, neighbour_chunk_index));
          return generate_chunk_info(prng_global, prng_local, m_config, neighbour_chunk_index);
        });
        assert(success);
      }

      if(!it->second.try_get())
        can_load = false;
    }

  if(!can_load)
    return;

  // 2: Chunk Generation
  auto [it, success] = world.dimension.chunks.try_emplace(chunk_index);
  assert(success);
  Chunk& chunk = it->second;

  const ChunkInfo& chunk_info = m_chunk_infos.at(chunk_index).get();

  // 2.1: Create terrain based on height maps
  for(int z=0; z<CHUNK_HEIGHT; ++z)
    for(int y=0; y<CHUNK_WIDTH; ++y)
      for(int x=0; x<CHUNK_WIDTH; ++x)
      {
        Block *block = get_block(chunk, glm::ivec3(x, y, z));
        if(!block) [[unlikely]]
          continue;

        float height = 0.0f;
        for(size_t i=0; i<chunk_info.height_maps.size(); ++i)
        {
          const HeightMap& height_map = chunk_info.height_maps[i];

          height += height_map.heights[y][x];
          if(z < height)
          {
            block->id          = m_config.terrain.layers[i].block_id;
            block->light_level = 0;
            block->sky         = false;
            goto done;
          }
        }

        block->id          = BLOCK_ID_NONE;
        block->light_level = 15;
        block->sky         = true;
done:;
      }

  // 2.2: Carve out caves based off worms
  for(int y = chunk_index.y - radius; y <= chunk_index.y + radius; ++y)
    for(int x = chunk_index.x - radius; x <= chunk_index.x + radius; ++x)
    {
      glm::ivec2       neighbour_chunk_index = glm::ivec2(x, y);
      const ChunkInfo& neighbour_chunk_info  = m_chunk_infos.at(neighbour_chunk_index).get();
      for(const Worm& worm : neighbour_chunk_info.worms)
        for(const Worm::Node& node : worm.nodes)
        {
          glm::vec3  center  = coordinates::global_to_local(node.center, chunk_index);
          float      radius  = node.radius;
          for(int z = center.z - radius; z<=center.z+radius; ++z)
            for(int y = center.y - radius; y<=center.y+radius; ++y)
              for(int x = center.x - radius; x<=center.x+radius; ++x)
              {
                glm::ivec3 position(x, y, z);
                if(glm::length2(glm::vec3(position) - center) < radius * radius)
                  if(Block* block = get_block(chunk, position))
                  {
                    block->id          = BLOCK_ID_NONE;
                    block->light_level = 0;
                    block->sky         = false;
                    light_manager.invalidate(coordinates::local_to_global(position, chunk_index));
                  }
              }
        }
    }

  chunk.mesh_invalidated = true;
}

