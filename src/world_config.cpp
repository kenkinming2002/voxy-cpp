#include <world_config.hpp>

#include <yaml-cpp/yaml.h>
#include <fmt/format.h>

static GenerationConfig load_generation_config(std::string_view path)
{
  GenerationConfig generation_config;

  YAML::Node root = YAML::LoadFile(fmt::format("{}/configs/generation.yaml", path));
  YAML::Node terrain = root["terrain"];

  YAML::Node layers = terrain["layers"];
  for(YAML::Node layer : layers)
  {
    LayerGenerationConfig layer_generation_config;

    layer_generation_config.block_id = layer["block_id"].as<std::uint32_t>();

    layer_generation_config.height_base              = layer["height_base"]                .as<float>();
    layer_generation_config.height_noise.frequency   = layer["height_noise"]["frequency"]  .as<float>();
    layer_generation_config.height_noise.amplitude   = layer["height_noise"]["amplitude"]  .as<float>();
    layer_generation_config.height_noise.lacunarity  = layer["height_noise"]["lacunarity"] .as<float>();
    layer_generation_config.height_noise.persistence = layer["height_noise"]["persistence"].as<float>();
    layer_generation_config.height_noise.octaves     = layer["height_noise"]["octaves"]    .as<unsigned>();

    generation_config.terrain.layers.push_back(layer_generation_config);
  }

  YAML::Node caves = terrain["caves"];

  generation_config.caves.max_per_chunk      = caves["max_per_chunk"]     .as<unsigned>();
  generation_config.caves.max_segment        = caves["max_segment"]       .as<unsigned>();
  generation_config.caves.step               = caves["step"]              .as<float>();
  generation_config.caves.min_height         = caves["min_height"]        .as<float>();
  generation_config.caves.max_height         = caves["max_height"]        .as<float>();

  generation_config.caves.dig_noise.frequency   = caves["dig_noise"]["frequency"]     .as<float>();
  generation_config.caves.dig_noise.amplitude   = caves["dig_noise"]["amplitude"]     .as<float>();
  generation_config.caves.dig_noise.lacunarity  = caves["dig_noise"]["lacunarity"]    .as<float>();
  generation_config.caves.dig_noise.persistence = caves["dig_noise"]["persistence"]   .as<float>();
  generation_config.caves.dig_noise.octaves     = caves["dig_noise"]["octaves"]       .as<unsigned>();

  generation_config.caves.radius_base              = caves["radius_base"]                .as<float>();
  generation_config.caves.radius_noise.frequency   = caves["radius_noise"]["frequency"]  .as<float>();
  generation_config.caves.radius_noise.amplitude   = caves["radius_noise"]["amplitude"]  .as<float>();
  generation_config.caves.radius_noise.lacunarity  = caves["radius_noise"]["lacunarity"] .as<float>();
  generation_config.caves.radius_noise.persistence = caves["radius_noise"]["persistence"].as<float>();
  generation_config.caves.radius_noise.octaves     = caves["radius_noise"]["octaves"]    .as<unsigned>();

  return generation_config;
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
    .generation = load_generation_config(path),
    .blocks     = load_block_configs(path),
    .entities   = load_entity_configs(path),
  };
}

