#pragma once

#include <graphics/texture.hpp>
#include <graphics/ui_renderer.hpp>

#include <glm/glm.hpp>

#include <memory>

namespace graphics
{
  struct Font
  {
  public:
    Font(const char *font, unsigned height);

  public:
    void render(UIRenderer& renderer, glm::vec2 viewport, glm::vec2 position, const char* str);

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
}
