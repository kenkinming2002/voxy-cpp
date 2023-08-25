#pragma once

#include <graphics/mesh.hpp>
#include <graphics/texture.hpp>
#include <graphics/texture_array.hpp>

#include <vector>
#include <memory>
#include <string_view>

#include <cstdint>

struct BlockResource
{
  std::uint32_t texture_indices[6];
};

struct EntityResource
{
  std::unique_ptr<graphics::Mesh>    mesh;
  std::unique_ptr<graphics::Texture> texture;
};

struct ResourcePack
{
  std::vector<BlockResource>  blocks;
  std::vector<EntityResource> entities;

  std::unique_ptr<graphics::TextureArray> blocks_texture_array;
};

ResourcePack load_resource_pack(std::string_view path);
