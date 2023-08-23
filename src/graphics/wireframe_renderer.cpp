#include <graphics/wireframe_renderer.hpp>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace graphics
{
  WireframeRenderer::WireframeRenderer()
  {
    m_cube_shader_program = std::make_unique<graphics::ShaderProgram>("assets/ui_selection.vert", "assets/ui_selection.frag");

    std::vector<uint8_t>  indices;
    std::vector<glm::vec3> vertices;

    indices.push_back(0); indices.push_back(1);
    indices.push_back(2); indices.push_back(3);
    indices.push_back(4); indices.push_back(5);
    indices.push_back(6); indices.push_back(7);

    indices.push_back(0); indices.push_back(2);
    indices.push_back(4); indices.push_back(6);
    indices.push_back(1); indices.push_back(3);
    indices.push_back(5); indices.push_back(7);

    indices.push_back(0); indices.push_back(4);
    indices.push_back(1); indices.push_back(5);
    indices.push_back(2); indices.push_back(6);
    indices.push_back(3); indices.push_back(7);

    for(int z : {0, 1})
      for(int y : {0, 1})
        for(int x : {0, 1})
          vertices.push_back(glm::vec3(x, y, z));

    graphics::MeshLayout layout{
      .index_type = graphics::IndexType::UNSIGNED_BYTE,
        .stride = sizeof(glm::vec3),
        .attributes = {
          { .type = graphics::AttributeType::FLOAT3, .offset = 0, },
        },
    };

    m_cube_mesh = std::make_unique<graphics::Mesh>(
        std::move(layout),
        graphics::as_bytes(indices),
        graphics::as_bytes(vertices)
        );
  }
}

void graphics::WireframeRenderer::render_cube(const Camera& camera, glm::vec3 position, glm::vec3 dimension, float thickness)
{
  glm::mat4 view       = camera.view();
  glm::mat4 projection = camera.projection();

  glm::mat4 model = glm::mat4(1.0f);
  model = glm::translate(model, position);
  model = glm::scale    (model, dimension);

  glm::mat4 MVP = projection * view * model;

  glUseProgram(m_cube_shader_program->id());
  glUniformMatrix4fv(glGetUniformLocation(m_cube_shader_program->id(), "MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
  glLineWidth(thickness);
  m_cube_mesh->draw_lines();
}
