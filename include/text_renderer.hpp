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

struct Font
{
public:
  Font(const char *font, unsigned height);

private:
  friend class TextRenderer;

private:
  struct Glyph
  {
    glm::ivec3 dimenson;
    glm::vec2  bearing;
    glm::vec2  advance;

    std::unique_ptr<graphics::Texture> texture;
  };
  Glyph m_glyphs[128];
};

class TextRenderer
{
public:
  TextRenderer();

public:
  void render(glm::vec2 dimension, glm::vec2 position, const Font& font, const char* str);

private:
  std::unique_ptr<graphics::ShaderProgram> m_shader_program;
  std::unique_ptr<graphics::Mesh>          m_quad_mesh;
};

#endif // TEXT_RENDERER_HPP
