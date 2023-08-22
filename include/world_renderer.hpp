#pragma once

#include <world.hpp>
#include <world_config.hpp>

#include <camera.hpp>

#include <graphics/shader_program.hpp>
#include <graphics/texture_array.hpp>
#include <graphics/mesh.hpp>
#include <graphics/texture.hpp>

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <unordered_map>
#include <memory>

#include <cstdint>

struct BlockRenderInfo
{
  std::uint32_t texture_indices[6];
};

struct EntityRenderInfo
{
  std::unique_ptr<graphics::Mesh>    mesh;
  std::unique_ptr<graphics::Texture> texture;
};

class WorldRenderer
{
public:
  static constexpr double REMASH_THROTTLE = 5.0f;

public:
  WorldRenderer(const WorldConfig& config);

public:
  void render(const Camera& camera, const World& world, bool third_person);

private:
  void render_chunks(const Camera& camera, const World& world);
  void render_entites(const Camera& camera, const World& world, bool third_person);

private:
  std::vector<BlockRenderInfo> m_block_render_infos;

  std::unique_ptr<graphics::ShaderProgram>       m_chunk_shader_program;
  std::unique_ptr<graphics::TextureArray>        m_chunk_texture_array;
  std::unordered_map<glm::ivec2, graphics::Mesh> m_chunk_meshes;

  std::vector<EntityRenderInfo> m_entity_render_infos;

  std::unique_ptr<graphics::ShaderProgram> m_entity_shader_program;
};
