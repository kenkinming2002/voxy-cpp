#include <world_config.hpp>

#include <yaml-cpp/yaml.h>
#include <fmt/format.h>

WorldConfig load_world_config(std::string_view path)
{
  WorldConfig world_config;

  YAML::Node root = YAML::LoadFile(fmt::format("{}/config.yaml", path));
  YAML::Node generation = root["generation"];

  YAML::Node seed = generation["seed"];
  world_config.generation.seed = seed.as<std::size_t>();

  YAML::Node terrain = generation["terrain"];

  for(YAML::Node layer : terrain["layers"])
  {
    LayerGenerationConfig layer_generation_config;

    layer_generation_config.block_id = layer["block_id"].as<std::uint32_t>();

    layer_generation_config.height_base              = layer["height_base"]                .as<float>();
    layer_generation_config.height_noise.frequency   = layer["height_noise"]["frequency"]  .as<float>();
    layer_generation_config.height_noise.amplitude   = layer["height_noise"]["amplitude"]  .as<float>();
    layer_generation_config.height_noise.lacunarity  = layer["height_noise"]["lacunarity"] .as<float>();
    layer_generation_config.height_noise.persistence = layer["height_noise"]["persistence"].as<float>();
    layer_generation_config.height_noise.octaves     = layer["height_noise"]["octaves"]    .as<unsigned>();

    world_config.generation.terrain.layers.push_back(layer_generation_config);
  }

  YAML::Node caves = generation["caves"];

  world_config.generation.caves.max_per_chunk = caves["max_per_chunk"]     .as<unsigned>();
  world_config.generation.caves.max_segment   = caves["max_segment"]       .as<unsigned>();
  world_config.generation.caves.step          = caves["step"]              .as<float>();
  world_config.generation.caves.min_height    = caves["min_height"]        .as<float>();
  world_config.generation.caves.max_height    = caves["max_height"]        .as<float>();

  world_config.generation.caves.dig_noise.frequency   = caves["dig_noise"]["frequency"]     .as<float>();
  world_config.generation.caves.dig_noise.amplitude   = caves["dig_noise"]["amplitude"]     .as<float>();
  world_config.generation.caves.dig_noise.lacunarity  = caves["dig_noise"]["lacunarity"]    .as<float>();
  world_config.generation.caves.dig_noise.persistence = caves["dig_noise"]["persistence"]   .as<float>();
  world_config.generation.caves.dig_noise.octaves     = caves["dig_noise"]["octaves"]       .as<unsigned>();

  world_config.generation.caves.radius_base              = caves["radius_base"]                .as<float>();
  world_config.generation.caves.radius_noise.frequency   = caves["radius_noise"]["frequency"]  .as<float>();
  world_config.generation.caves.radius_noise.amplitude   = caves["radius_noise"]["amplitude"]  .as<float>();
  world_config.generation.caves.radius_noise.lacunarity  = caves["radius_noise"]["lacunarity"] .as<float>();
  world_config.generation.caves.radius_noise.persistence = caves["radius_noise"]["persistence"].as<float>();
  world_config.generation.caves.radius_noise.octaves     = caves["radius_noise"]["octaves"]    .as<unsigned>();

  return world_config;
}

