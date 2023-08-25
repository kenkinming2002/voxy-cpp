#pragma once

#include <noise.hpp>

#include <array>
#include <string>
#include <vector>

#include <cstddef>
#include <cstdint>

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

struct GenerationConfig
{
  std::size_t             seed;
  TerrainGenerationConfig terrain;
  CavesGenerationConfig   caves;
};

struct BlockConfig
{
  std::array<std::string, 6> textures;
};

struct EntityConfig
{
  std::string model;
  std::string texture;
};

struct WorldConfig
{
  GenerationConfig          generation;
  std::vector<BlockConfig>  blocks;
  std::vector<EntityConfig> entities;
};

WorldConfig load_world_config(std::string_view path);
