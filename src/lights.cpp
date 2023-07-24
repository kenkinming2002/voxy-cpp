#include <lights.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Lights::Lights(Light light) :
  program(gl::compile_program("assets/light.vert", "assets/light.frag")),
  light(std::move(light))
{}

void Lights::update(float dt)
{
  light.pos.x += 5.0f * dt;
}

void Lights::render(const Camera& camera)
{
  glm::mat4 view       = camera.view();
  glm::mat4 projection = camera.projection();

  // 1: Light
  glUseProgram(program);
  {
    glm::mat4 model  = glm::translate(glm::mat4(1.0f), light.pos);
    glm::mat4 MVP    = projection * view * model;
    glUniformMatrix4fv(glGetUniformLocation(program, "MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
    light.mesh.draw();
  }
}
