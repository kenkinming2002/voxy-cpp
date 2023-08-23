#include <graphics/wireframe_renderer.hpp>

#include <glm/gtc/matrix_transform.hpp>

namespace graphics
{
  WireframeRenderer::WireframeRenderer()
  {
    m_shader_program = std::make_unique<graphics::ShaderProgram>("assets/wireframe.vert", "assets/wireframe.frag");

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

    const Attribute attributes[] = {{ .type = graphics::AttributeType::FLOAT3, .offset = 0, }};
    m_cube_mesh = std::make_unique<Mesh>(
        IndexType::UNSIGNED_BYTE,
        PrimitiveType::LINES,
        sizeof(glm::vec3),
        attributes
    );
    m_cube_mesh->write(std::as_bytes(std::span(indices)), std::as_bytes(std::span(vertices)), Usage::STATIC);
  }

  void WireframeRenderer::render_cube(const Camera& camera, glm::vec3 position, glm::vec3 dimension, glm::vec3 color, float thickness)
  {
    glm::mat4 view       = camera.view();
    glm::mat4 projection = camera.projection();

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::scale    (model, dimension);

    m_shader_program->use();
    m_shader_program->set_uniform("MVP",   projection * view * model);
    m_shader_program->set_uniform("color", color);

    glLineWidth(thickness);
    m_cube_mesh->draw();
  }
}

