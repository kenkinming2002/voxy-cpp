#pragma once

#include <graphics/shader_program.hpp>
#include <graphics/mesh.hpp>

#include <camera.hpp>

#include <memory>

namespace graphics
{
  class WireframeRenderer
  {
  public:
    WireframeRenderer();

  public:
    void render_cube(const Camera& camera, glm::vec3 position, glm::vec3 dimension, float thickness);

  private:
    std::unique_ptr<ShaderProgram> m_cube_shader_program;
    std::unique_ptr<Mesh>          m_cube_mesh;
  };
}
