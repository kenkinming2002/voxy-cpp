#include <system/debug.hpp>

#include <world.hpp>

#include <text_renderer.hpp>

#include <fmt/format.h>

static constexpr glm::vec2   DEBUG_MARGIN       = glm::vec2(3.0f, 3.0f);
static constexpr const char *DEBUG_FONT        = "assets/arial.ttf";
static constexpr float       DEBUG_FONT_HEIGHT = 20.0f;

class DebugSystemImpl : public DebugSystem
{
public:
  DebugSystemImpl() : m_text_renderer(DEBUG_FONT, DEBUG_FONT_HEIGHT) { }

private:
  void render(const World& world) override
  {
    std::string line;
    glm::vec2   cursor = DEBUG_MARGIN;

    line = fmt::format("position: x = {}, y = {}, z = {}", world.player().transform.position.x, world.player().transform.position.y, world.player().transform.position.z);
    m_text_renderer.render(cursor, line.c_str());
    cursor.x = DEBUG_MARGIN.x;
    cursor.y += DEBUG_FONT_HEIGHT;

    line = fmt::format("velocity: x = {}, y = {}, z = {}", world.player().velocity.x, world.player().velocity.y, world.player().velocity.z);
    m_text_renderer.render(cursor, line.c_str());
    cursor.x = DEBUG_MARGIN.x;
    cursor.y += DEBUG_FONT_HEIGHT;
  }

private:
  TextRenderer m_text_renderer;
};

std::unique_ptr<DebugSystem> DebugSystem::create()
{
  return std::make_unique<DebugSystemImpl>();
}


