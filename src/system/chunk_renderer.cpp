#include <system/chunk_renderer.hpp>

#include <world.hpp>

#include <glm/gtc/type_ptr.hpp>

class ChunkRendererSystemImpl : public ChunkRendererSystem
{
public:
  ChunkRendererSystemImpl()
    : m_program(gl::compile_program("assets/chunk.vert", "assets/chunk.frag")) {}

private:
  void render(const World& world) override
  {
    glm::mat4 view       = world.camera().view();
    glm::mat4 projection = world.camera().projection();
    glUseProgram(m_program);
    {
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D_ARRAY, world.dimension().blocks_texture_array.id());
      glUniform1i(glGetUniformLocation(m_program, "blocksTextureArray"), 0);
      for(const auto& [chunk_index, chunk] : world.dimension().chunks)
      {
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 MVP   = projection * view * model;
        glUniformMatrix4fv(glGetUniformLocation(m_program, "MVP"),    1, GL_FALSE, glm::value_ptr(MVP));
        if(chunk.mesh)
          chunk.mesh->draw();
      }
    }
  }

private:
  gl::Program m_program;
};

std::unique_ptr<ChunkRendererSystem> ChunkRendererSystem::create()
{
  return std::make_unique<ChunkRendererSystemImpl>();
}
