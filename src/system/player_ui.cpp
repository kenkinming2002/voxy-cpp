#include <system/player_ui.hpp>

#include <types/world.hpp>
#include <types/directions.hpp>

#include <mesh.hpp>
#include <gl.hpp>

#include <glm/gtc/type_ptr.hpp>

static constexpr float UI_SELECTION_THICKNESS = 3.0f;

static Mesh build_unit_cube_mesh()
{
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

  MeshLayout layout{
    .index_type = IndexType::UNSIGNED_BYTE,
      .stride = sizeof(glm::vec3),
      .attributes = {
        { .type = AttributeType::FLOAT3, .offset = 0, },
      },
  };

  return Mesh(
    std::move(layout),
    as_bytes(indices),
    as_bytes(vertices)
  );
}

class UIRendererSystemImpl : public PlayerUISystem
{
public:
  UIRendererSystemImpl()
    : m_program(gl::compile_program("assets/ui_selection.vert", "assets/ui_selection.frag")),
      m_mesh(build_unit_cube_mesh()) {}

private:
  void render(const World& world) override
  {
    // 1: Selection
    if(world.selection)
    {
      glm::vec3 position = *world.selection;

      glm::mat4 view       = world.camera.view();
      glm::mat4 projection = world.camera.projection();
      glm::mat4 model      = glm::translate(glm::mat4(1.0f), position);
      glm::mat4 MVP        = projection * view * model;

      glUseProgram(m_program);
      glUniformMatrix4fv(glGetUniformLocation(m_program, "MVP"), 1, GL_FALSE, glm::value_ptr(MVP));

      glLineWidth(UI_SELECTION_THICKNESS);
      m_mesh.draw_lines();
    }
  }

private:
  gl::Program m_program;
  Mesh        m_mesh;
};

std::unique_ptr<PlayerUISystem> PlayerUISystem::create()
{
  return std::make_unique<UIRendererSystemImpl>();
}


