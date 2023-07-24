#include <terrain.hpp>

#include <glm/gtc/type_ptr.hpp>

Terrain::Terrain() :
  program(gl::compile_program("assets/chunk.vert", "assets/chunk.frag")),
  chunks()
{}

void Terrain::load(glm::vec2 center, float radius)
{
  glm::ivec2 ccenter = center / (float)Layer::WIDTH;
  int        cradius = radius / Layer::WIDTH;

  for(int yoff = -cradius; yoff <= cradius; ++yoff)
    for(int xoff = -cradius; xoff <= cradius; ++xoff)
    {
      glm::ivec2 coff(xoff, yoff);
      if(xoff * xoff + yoff * yoff < cradius)
      {
        glm::ivec2 cpos = ccenter + coff;
        if(!chunks.contains(cpos))
          chunks.emplace(std::piecewise_construct,
            std::forward_as_tuple(cpos),
            std::forward_as_tuple(cpos)
          );
      }
    }
}

void Terrain::unload(glm::vec2 center, float radius)
{
  glm::ivec2 ccenter = center / (float)Layer::WIDTH;
  int        cradius = radius / Layer::WIDTH;

  for(auto it = chunks.begin(); it != chunks.end();)
  {
    const auto& [cpos, chunk_mesh] = *it;
    glm::ivec2 coff = cpos - ccenter;
    if(coff.x * coff.x + coff.y * coff.y > cradius)
      it = chunks.erase(it);
    else
      ++it;
  }
}

void Terrain::update(float dt)
{
  // Nothing to do for now
}

void Terrain::render(const Camera& camera, const Light& light)
{
  glm::mat4 view       = camera.view();
  glm::mat4 projection = camera.projection();

  glUseProgram(program);
  {
    glUniform3fv(glGetUniformLocation(program, "viewPos"),        1, glm::value_ptr(camera.position));
    glUniform3fv(glGetUniformLocation(program, "light.pos"),      1, glm::value_ptr(light.pos));
    glUniform3fv(glGetUniformLocation(program, "light.ambient"),  1, glm::value_ptr(light.ambient));
    glUniform3fv(glGetUniformLocation(program, "light.diffuse"),  1, glm::value_ptr(light.diffuse));

    for(const auto& [cpos, chunk] : chunks)
    {
      glm::mat4 model  = glm::translate(glm::mat4(1.0f), glm::vec3( Layer::WIDTH * cpos.x, Layer::WIDTH * cpos.y, 0.0f));
      glm::mat4 normal = glm::transpose(glm::inverse(model));
      glm::mat4 MVP    = projection * view * model;

      glUniformMatrix4fv(glGetUniformLocation(program, "MVP"),    1, GL_FALSE, glm::value_ptr(MVP));
      glUniformMatrix4fv(glGetUniformLocation(program, "model"),  1, GL_FALSE, glm::value_ptr(model));
      glUniformMatrix4fv(glGetUniformLocation(program, "normal"), 1, GL_FALSE, glm::value_ptr(normal));

      chunk.layers_mesh.draw();
    }
  }
}

