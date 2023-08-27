#pragma once

#include <world.hpp>

#include <light_manager.hpp>

#include <noise.hpp>
#include <lazy.hpp>

#include <unordered_map>

struct LayerGenerationConfig
{
  std::uint32_t block_id;

  float       height_base;
  NoiseConfig height_noise;
};

struct TerrainGenerationConfig
{
  std::vector<LayerGenerationConfig> layers;
};

struct CavesGenerationConfig
{
  unsigned max_per_chunk;
  unsigned max_segment;
  float    step;

  float min_height;
  float max_height;

  NoiseConfig dig_noise;

  float       radius_base;
  NoiseConfig radius_noise;
};

struct WorldGenerationConfig
{
  std::size_t             seed;
  TerrainGenerationConfig terrain;
  CavesGenerationConfig   caves;
};

WorldGenerationConfig load_world_generation_config(std::string_view path);

class WorldGenerator
{
public:
  static constexpr size_t CHUNK_LOAD_RADIUS = 4;

public:
  WorldGenerator(WorldGenerationConfig config);

public:
  void update(World& world, LightManager& light_manager);

private:
  void try_load(World& world, LightManager& light_manager, glm::ivec2 chunk_index, int radius);
  void try_load(World& world, LightManager& light_manager, glm::ivec2 chunk_index);

private:
  WorldGenerationConfig m_config;

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
  template<typename Prng> static std::vector<HeightMap> generate_height_maps(Prng& prng, const TerrainGenerationConfig& config, glm::ivec2 chunk_index);
  template<typename Prng> static std::vector<Worm> generate_worms(Prng& prng, const CavesGenerationConfig& config, glm::ivec2 chunk_index);
  template<typename Prng> static ChunkInfo generate_chunk_info(Prng& prng_global, Prng& prng_local, const WorldGenerationConfig& config, glm::ivec2 chunk_index);

private:
  std::unordered_map<glm::ivec2, Lazy<ChunkInfo>> m_chunk_infos;
};
