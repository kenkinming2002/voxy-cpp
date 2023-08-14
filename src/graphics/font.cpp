#include <graphics/font.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

namespace graphics
{
  Font::Font(const char *font, unsigned height)
  {
    FT_Library library;
    FT_Face    face;

    if(FT_Init_FreeType(&library) != 0)
      throw std::runtime_error("Failed to initialize freetype");

    if(FT_New_Face(library, font, 0, &face) != 0)
      throw std::runtime_error("Failed to load font");

    if(FT_Set_Pixel_Sizes(face, 0, height) != 0)
      throw std::runtime_error("Failed to set pixel sizes");

    for(int c = 0; c<128; ++c)
    {
      if(FT_Load_Char(face, c, FT_LOAD_RENDER) != 0)
        throw std::runtime_error("Failed to load character");

      m_glyphs[c].dimenson.x = face->glyph->bitmap.width;
      m_glyphs[c].dimenson.y = face->glyph->bitmap.rows;

      m_glyphs[c].bearing.x = face->glyph->bitmap_left;
      m_glyphs[c].bearing.y = -(int)face->glyph->bitmap.rows+(int)face->glyph->bitmap_top;

      m_glyphs[c].advance.x = face->glyph->advance.x / 64.0f;
      m_glyphs[c].advance.y = face->glyph->advance.y / 64.0f;

      m_glyphs[c].texture = std::make_unique<graphics::Texture>(face->glyph->bitmap.buffer, face->glyph->bitmap.width, face->glyph->bitmap.rows, 1);
    }

    FT_Done_Face(face);
    FT_Done_FreeType(library);
  }

  void Font::render(UIRenderer& renderer, glm::vec2 viewport, glm::vec2 position, const char* str)
  {
    for(const char *it = str; *it; ++it)
    {
      int c = *it;
      assert(c >= 0 && c < 128);

      const Glyph& glyph = m_glyphs[c];
      renderer.render(viewport, position + glyph.bearing, glyph.dimenson, *glyph.texture);
      position += glyph.advance;
    }
  }
}
