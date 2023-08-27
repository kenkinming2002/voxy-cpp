#pragma once

#include <graphics/window.hpp>
#include <graphics/ui_renderer.hpp>
#include <graphics/font.hpp>

#include <world.hpp>

class DebugRenderer
{
public:
  static constexpr glm::vec2   DEBUG_MARGIN       = glm::vec2(3.0f, 3.0f);
  static constexpr const char *DEBUG_FONT        = "assets/arial.ttf";
  static constexpr float       DEBUG_FONT_HEIGHT = 20.0f;

  static constexpr size_t DT_AVERAGE_COUNT = 32;

public:
  DebugRenderer();

public:
  void update(float dt);
  void render(glm::vec2 viewport, const World& world);

private:
  void render_line(glm::vec2 viewport, size_t n, const std::string& line);

private:
  graphics::UIRenderer            m_ui_renderer;
  std::unique_ptr<graphics::Font> m_font;

  float m_dts[DT_AVERAGE_COUNT];
};
