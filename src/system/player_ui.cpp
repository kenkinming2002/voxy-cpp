#include <system/player_ui.hpp>

#include <world.hpp>

#include <graphics/mesh.hpp>
#include <graphics/shader_program.hpp>

#include <glm/gtc/type_ptr.hpp>

static constexpr float UI_SELECTION_THICKNESS = 3.0f;

static graphics::Mesh build_unit_cube_mesh()
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

  graphics::MeshLayout layout{
    .index_type = graphics::IndexType::UNSIGNED_BYTE,
      .stride = sizeof(glm::vec3),
      .attributes = {
        { .type = graphics::AttributeType::FLOAT3, .offset = 0, },
      },
  };

  return graphics::Mesh(
    std::move(layout),
    graphics::as_bytes(indices),
    graphics::as_bytes(vertices)
  );
}

class PlayerUISystem : public System
{
public:
  PlayerUISystem()
    : m_shader_program("assets/ui_selection.vert", "assets/ui_selection.frag"),
      m_mesh(build_unit_cube_mesh()) {}

private:
  void on_render(Application& application, const WorldConfig& world_config, const WorldData& world_data, const Camera& camera) override
  {
    // 1: Selection
    if(world_data.selection)
    {
      glm::vec3 position = *world_data.selection;

      glm::mat4 view       = camera.view();
      glm::mat4 projection = camera.projection();
      glm::mat4 model      = glm::translate(glm::mat4(1.0f), position);
      glm::mat4 MVP        = projection * view * model;

      glUseProgram(m_shader_program.id());
      glUniformMatrix4fv(glGetUniformLocation(m_shader_program.id(), "MVP"), 1, GL_FALSE, glm::value_ptr(MVP));

      glLineWidth(UI_SELECTION_THICKNESS);
      m_mesh.draw_lines();
    }

    // 1: Selection
    if(world_data.placement)
    {
      glm::vec3 position = *world_data.placement;

      glm::mat4 view       = camera.view();
      glm::mat4 projection = camera.projection();
      glm::mat4 model      = glm::translate(glm::mat4(1.0f), position);
      glm::mat4 MVP        = projection * view * model;

      glUseProgram(m_shader_program.id());
      glUniformMatrix4fv(glGetUniformLocation(m_shader_program.id(), "MVP"), 1, GL_FALSE, glm::value_ptr(MVP));

      glLineWidth(UI_SELECTION_THICKNESS);
      m_mesh.draw_lines();
    }
  }

private:
  graphics::ShaderProgram m_shader_program;
  graphics::Mesh          m_mesh;
};

std::unique_ptr<System> create_player_ui_system()
{
  return std::make_unique<PlayerUISystem>();
}


