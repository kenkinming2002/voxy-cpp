#include <graphics/ui_renderer.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace graphics
{
  UIRenderer::UIRenderer()
  {
    struct Vertex
    {
      glm::vec2 position;
      glm::vec2 tex_coords;
    };

    m_shader_program = std::make_unique<graphics::ShaderProgram>("./assets/ui.vert", "./assets/ui.frag");
    m_quad_mesh = std::make_unique<graphics::Mesh>(
      graphics::MeshLayout{
        .index_type = graphics::IndexType::UNSIGNED_BYTE,
        .stride     = sizeof(Vertex),
        .attributes = {
          { .type = graphics::AttributeType::FLOAT2, .offset = offsetof(Vertex, position),   },
          { .type = graphics::AttributeType::FLOAT2, .offset = offsetof(Vertex, tex_coords), },
        }
      },
      graphics::as_bytes(std::vector<uint8_t>{0, 1, 2, 2, 1, 3}),
      graphics::as_bytes(std::vector<Vertex>{
        { .position = glm::vec2(0.0f, 0.0f), .tex_coords = glm::vec2(0.0f, 1.0f), },
        { .position = glm::vec2(1.0f, 0.0f), .tex_coords = glm::vec2(1.0f, 1.0f), },
        { .position = glm::vec2(0.0f, 1.0f), .tex_coords = glm::vec2(0.0f, 0.0f), },
        { .position = glm::vec2(1.0f, 1.0f), .tex_coords = glm::vec2(1.0f, 0.0f), },
      })
    );
  }

  void UIRenderer::render(glm::vec2 viewport, glm::vec2 position, glm::vec2 dimension, const graphics::Texture& texture)
  {
    glDisable(GL_DEPTH_TEST);
    glUseProgram(m_shader_program->id());

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture.id());
    glUniform1i(glGetUniformLocation(m_shader_program->id(), "ourTexture"), 0);

    glm::mat4 projection = glm::ortho(0.0f, (float)viewport.x, 0.0f, (float)viewport.y);
    glm::mat4 view       = glm::mat4(1.0f);

    glm::mat4 model  = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(position .x, position .y, 0.0f));
    model = glm::scale    (model, glm::vec3(dimension.x, dimension.y, 0.0f));

    glm::mat4 MVP = projection * view * model;
    glUniformMatrix4fv(glGetUniformLocation(m_shader_program->id(), "MVP"), 1, GL_FALSE, glm::value_ptr(MVP));

    m_quad_mesh->draw_triangles();

    glEnable(GL_DEPTH_TEST);
  }
}
