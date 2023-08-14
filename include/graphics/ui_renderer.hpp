#pragma once

#include <graphics/shader_program.hpp>
#include <graphics/mesh.hpp>
#include <graphics/texture.hpp>

#include <memory>

namespace graphics
{
  class UIRenderer
  {
  public:
    UIRenderer();

  public:
    void render(glm::vec2 viewport, glm::vec2 position, glm::vec2 dimension, const graphics::Texture& texture);

  private:
    std::unique_ptr<graphics::ShaderProgram> m_shader_program;
    std::unique_ptr<graphics::Mesh>          m_quad_mesh;
  };
}
