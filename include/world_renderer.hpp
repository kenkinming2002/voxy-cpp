#pragma once

#include <world.hpp>
#include <world_config.hpp>

#include <camera.hpp>

#include <graphics/shader_program.hpp>
#include <graphics/texture_array.hpp>
#include <graphics/mesh.hpp>

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <unordered_map>
#include <memory>

#include <cstdint>

struct BlockRenderInfo
{
  std::uint32_t texture_indices[6];
};

class WorldRenderer
{
public:
  static constexpr double REMASH_THROTTLE = 5.0f;

public:
  WorldRenderer(const WorldConfig& config);

public:
  void render(const Camera& camera, const World& world);

private:
  std::vector<BlockRenderInfo> m_block_render_infos;

  std::unique_ptr<graphics::ShaderProgram>       m_shader_program;
  std::unique_ptr<graphics::TextureArray>        m_texture_array;
  std::unordered_map<glm::ivec2, graphics::Mesh> m_chunk_meshes;
};
