#include <system/chunk_renderer.hpp>

#include <world.hpp>

#include <shader_program.hpp>

#include <glm/gtc/type_ptr.hpp>

class ChunkRendererSystemImpl : public ChunkRendererSystem
{
public:
  ChunkRendererSystemImpl() : m_shader_program("assets/chunk.vert", "assets/chunk.frag") {}

private:
  void render(const World& world) override
  {
    glUseProgram(m_shader_program.id());

    glm::mat4 view       = world.camera.view();
    glm::mat4 projection = world.camera.projection();
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 MVP   = projection * view * model;
    glUniformMatrix4fv(glGetUniformLocation(m_shader_program.id(), "MVP"), 1, GL_FALSE, glm::value_ptr(MVP));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, world.dimension.blocks_texture_array.id());
    glUniform1i(glGetUniformLocation(m_shader_program.id(), "blocksTextureArray"), 0);
    for(const auto& [chunk_index, chunk] : world.dimension.chunks)
      if(chunk.mesh)
        chunk.mesh->draw_triangles();
  }

private:
  ShaderProgram m_shader_program;
};

std::unique_ptr<ChunkRendererSystem> ChunkRendererSystem::create()
{
  return std::make_unique<ChunkRendererSystemImpl>();
}

