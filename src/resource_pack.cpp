#include <resource_pack.hpp>

#include <unordered_set>
#include <unordered_map>

#include <yaml-cpp/yaml.h>
#include <fmt/format.h>

ResourcePack load_resource_pack(std::string_view path)
{
  ResourcePack resource_pack;

  YAML::Node node = YAML::LoadFile(fmt::format("{}/manifest.yaml", path));

  // 1: Blocks
  YAML::Node blocks = node["blocks"];

  std::unordered_set<std::string>                block_texture_filenames_set;
  std::vector<std::string>                       block_texture_filenames;
  std::unordered_map<std::string, std::uint32_t> block_texture_indices_map;

  for(YAML::Node block : blocks)
  {
    YAML::Node textures = block["textures"];
    for(size_t i=0; i<6; ++i)
      block_texture_filenames_set.insert(textures[i].as<std::string>());
  }

  std::uint32_t i = 0;
  for(const std::string& block_texture_filename : block_texture_filenames_set)
  {
    block_texture_filenames.push_back(fmt::format("{}/{}", path, block_texture_filename));
    block_texture_indices_map.emplace(block_texture_filename, i++);
  }

  resource_pack.blocks_texture_array = std::make_unique<graphics::TextureArray>(block_texture_filenames);

  for(YAML::Node block : blocks)
  {
    YAML::Node textures = block["textures"];

    BlockResource block_resource;
    for(size_t i=0; i<6; ++i)
      block_resource.texture_indices[i] = block_texture_indices_map.at(textures[i].as<std::string>());
    resource_pack.blocks.push_back(block_resource);
  }

  // 2: Entities
  YAML::Node entities = node["entities"];
  for(YAML::Node entity : entities)
  {
    EntityResource entity_resource;
    entity_resource.mesh    = graphics::Mesh   ::load_from(fmt::format("{}/{}", path, entity["mesh"]   .as<std::string>()));
    entity_resource.texture = graphics::Texture::load_from(fmt::format("{}/{}", path, entity["texture"].as<std::string>()));
    resource_pack.entities.push_back(std::move(entity_resource));
  }

  return resource_pack;
}
