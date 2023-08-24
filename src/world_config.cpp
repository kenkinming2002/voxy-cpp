#include <world_config.hpp>

#include <yaml-cpp/yaml.h>
#include <fmt/format.h>

static WorldGenerationConfig load_world_generation_config(std::string_view path)
{
  WorldGenerationConfig world_generation_config;

  YAML::Node root = YAML::LoadFile(fmt::format("{}/configs/generation.yaml", path));
  YAML::Node terrain = root["terrain"];

  YAML::Node layers = terrain["layers"];
  for(YAML::Node layer : layers)
  {
    TerrainLayerGenerationConfig terrain_layer_generation_config;
    terrain_layer_generation_config.block_id     = layer["block_id"]   .as<std::uint32_t>();
    terrain_layer_generation_config.base         = layer["base"]       .as<float>();
    terrain_layer_generation_config.frequency    = layer["frequency"]  .as<float>();
    terrain_layer_generation_config.amplitude    = layer["amplitude"]  .as<float>();
    terrain_layer_generation_config.lacunarity   = layer["lacunarity"] .as<float>();
    terrain_layer_generation_config.persistence  = layer["persistence"].as<float>();
    terrain_layer_generation_config.octaves      = layer["octaves"]    .as<unsigned>();
    world_generation_config.terrain.layers.push_back(terrain_layer_generation_config);
  }

  YAML::Node caves = terrain["caves"];
  world_generation_config.caves.max_per_chunk      = caves["max_per_chunk"]     .as<unsigned>();
  world_generation_config.caves.max_segment        = caves["max_segment"]       .as<unsigned>();
  world_generation_config.caves.step               = caves["step"]              .as<float>();
  world_generation_config.caves.min_height         = caves["min_height"]        .as<float>();
  world_generation_config.caves.max_height         = caves["max_height"]        .as<float>();
  world_generation_config.caves.dig_frequency      = caves["dig_frequency"]     .as<float>();
  world_generation_config.caves.dig_amplitude      = caves["dig_amplitude"]     .as<float>();
  world_generation_config.caves.dig_lacunarity     = caves["dig_lacunarity"]    .as<float>();
  world_generation_config.caves.dig_persistence    = caves["dig_persistence"]   .as<float>();
  world_generation_config.caves.dig_octaves        = caves["dig_octaves"]       .as<float>();
  world_generation_config.caves.radius             = caves["radius"]            .as<float>();
  world_generation_config.caves.radius_frequency   = caves["radius_frequency"]  .as<float>();
  world_generation_config.caves.radius_amplitude   = caves["radius_amplitude"]  .as<float>();
  world_generation_config.caves.radius_lacunarity  = caves["radius_lacunarity"] .as<float>();
  world_generation_config.caves.radius_persistence = caves["radius_persistence"].as<float>();

  return world_generation_config;
}

static std::vector<BlockConfig> load_block_configs(std::string_view path)
{
  std::vector<BlockConfig> block_configs;

  YAML::Node root = YAML::LoadFile(fmt::format("{}/configs/blocks.yaml", path));
  for(YAML::Node block : root)
  {
    BlockConfig block_config;

    YAML::Node textures = block["textures"];
    for(size_t i=0; i<6; ++i)
      block_config.textures[i] = textures[i].as<std::string>();

    block_configs.push_back(block_config);
  }

  return block_configs;
}

static std::vector<EntityConfig> load_entity_configs(std::string_view path)
{
  std::vector<EntityConfig> entity_configs;

  YAML::Node root = YAML::LoadFile(fmt::format("{}/configs/entities.yaml", path));
  for(YAML::Node entity : root)
  {
    EntityConfig entity_config;
    entity_config.model   = entity["model"]  .as<std::string>();
    entity_config.texture = entity["texture"].as<std::string>();
    entity_configs.push_back(entity_config);
  }

  return entity_configs;
}

WorldConfig load_world_config(std::string_view path)
{
  return WorldConfig {
    .generation = load_world_generation_config(path),
    .blocks     = load_block_configs(path),
    .entities   = load_entity_configs(path),
  };
}

