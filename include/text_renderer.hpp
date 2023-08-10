#ifndef TEXT_RENDERER_HPP
#define TEXT_RENDERER_HPP

#include <graphics/shader_program.hpp>
#include <graphics/mesh.hpp>
#include <graphics/texture.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <glm/glm.hpp>
#include <glad/glad.h>

#include <memory>

class TextRenderer
{
public:
  TextRenderer(const char *font, unsigned height);

public:
  void render(glm::vec2& cursor, const char *str);

private:
  graphics::ShaderProgram m_shader_program;
  graphics::Mesh          m_quad_mesh;

  struct Glyph
  {
    glm::ivec3 dimenson;
    glm::vec2  bearing;
    glm::vec2  advance;

    std::unique_ptr<graphics::Texture> texture;
  };
  Glyph m_glyphs[128];
};

#endif // TEXT_RENDERER_HPP
