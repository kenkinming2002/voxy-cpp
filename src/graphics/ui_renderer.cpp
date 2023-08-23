#include <graphics/ui_renderer.hpp>

#include <glm/gtc/matrix_transform.hpp>

namespace graphics
{
  UIRenderer::UIRenderer()
  {
    m_shader_program = std::make_unique<graphics::ShaderProgram>("./assets/ui.vert", "./assets/ui.frag");

    struct Vertex
    {
      glm::vec2 position;
      glm::vec2 tex_coords;
    };

    const uint8_t indices[] = { 0, 1, 2, 2, 1, 3 };
    const Vertex vertices[] = {
      { .position = glm::vec2(0.0f, 0.0f), .tex_coords = glm::vec2(0.0f, 1.0f), },
      { .position = glm::vec2(1.0f, 0.0f), .tex_coords = glm::vec2(1.0f, 1.0f), },
      { .position = glm::vec2(0.0f, 1.0f), .tex_coords = glm::vec2(0.0f, 0.0f), },
      { .position = glm::vec2(1.0f, 1.0f), .tex_coords = glm::vec2(1.0f, 0.0f), },
    };

    const Attribute attributes[] = {
      { .type = graphics::AttributeType::FLOAT2, .offset = offsetof(Vertex, position),   },
      { .type = graphics::AttributeType::FLOAT2, .offset = offsetof(Vertex, tex_coords), },
    };
    m_quad_mesh = std::make_unique<Mesh>(IndexType::UNSIGNED_BYTE, PrimitiveType::TRIANGLES, sizeof(Vertex), attributes);
    m_quad_mesh->write(std::as_bytes(std::span(indices)), std::as_bytes(std::span(vertices)), Usage::STATIC);
  }

  void UIRenderer::render(glm::vec2 viewport, glm::vec2 position, glm::vec2 dimension, const graphics::Texture& texture)
  {
    glDisable(GL_DEPTH_TEST);

    m_shader_program->use();

    glm::mat4 projection = glm::ortho(0.0f, (float)viewport.x, 0.0f, (float)viewport.y);
    glm::mat4 view       = glm::mat4(1.0f);

    glm::mat4 model  = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(position .x, position .y, 0.0f));
    model = glm::scale    (model, glm::vec3(dimension.x, dimension.y, 0.0f));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture.id());
    m_shader_program->set_uniform("ourTexture", 0);

    m_shader_program->set_uniform("MVP", projection * view * model);
    m_quad_mesh->draw();

    glEnable(GL_DEPTH_TEST);
  }
}
