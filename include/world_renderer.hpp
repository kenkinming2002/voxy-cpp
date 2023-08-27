#pragma once

#include <world.hpp>

#include <resource_pack.hpp>

#include <graphics/camera.hpp>
#include <graphics/mesh.hpp>
#include <graphics/shader_program.hpp>
#include <graphics/texture.hpp>
#include <graphics/texture_array.hpp>
#include <graphics/wireframe_renderer.hpp>

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <unordered_map>
#include <memory>

#include <cstdint>

class WorldRenderer
{
public:
  static constexpr double REMASH_THROTTLE = 5.0f;

public:
  WorldRenderer(ResourcePack resource_pack);

public:
  void render(const graphics::Camera& camera, const World& world, bool third_person, graphics::WireframeRenderer& wireframe_renderer);

private:
  void render_chunks(const graphics::Camera& camera, const World& world);
  void render_entites(const graphics::Camera& camera, const World& world, bool third_person, graphics::WireframeRenderer& wireframe_renderer);

private:
  ResourcePack m_resource_pack;

  std::unique_ptr<graphics::ShaderProgram> m_chunk_shader_program;
  std::unique_ptr<graphics::ShaderProgram> m_entity_shader_program;

  std::unordered_map<glm::ivec2, std::unique_ptr<graphics::Mesh>> m_chunk_meshes;
};
