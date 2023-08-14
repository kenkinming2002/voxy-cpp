#include <system/debug.hpp>

#include <application.hpp>
#include <world.hpp>

#include <graphics/ui_renderer.hpp>
#include <graphics/font.hpp>

#include <fmt/format.h>

static constexpr glm::vec2   DEBUG_MARGIN       = glm::vec2(3.0f, 3.0f);
static constexpr const char *DEBUG_FONT        = "assets/arial.ttf";
static constexpr float       DEBUG_FONT_HEIGHT = 20.0f;

static constexpr size_t DT_AVERAGE_COUNT = 32;

class DebugSystem : public System
{
public:
  DebugSystem()
  {
    m_font = std::make_unique<graphics::Font>(DEBUG_FONT, DEBUG_FONT_HEIGHT);
    for(size_t i=0; i<DT_AVERAGE_COUNT; ++i)
      m_dts[i] = 0.0f;
  }

private:
  void on_update(Application& application, const WorldConfig& world_config, WorldData& world_data, float dt) override
  {
    for(size_t i=0; i<DT_AVERAGE_COUNT-1; ++i)
      m_dts[i] = m_dts[i+1];
    m_dts[DT_AVERAGE_COUNT-1] = dt;
  }

  void render_line(glm::vec2 viewport, size_t n, const std::string& line)
  {
    glm::vec2 position = DEBUG_MARGIN + glm::vec2(0.0f, n * DEBUG_FONT_HEIGHT);
    m_font->render(m_ui_renderer, viewport, position, line.c_str());
  }

  void on_render(Application& application, const WorldConfig& world_config, const WorldData& world_data) override
  {
    // 1: Frame time
    float average = 0.0f;
    for(size_t i=0; i<DT_AVERAGE_COUNT; ++i)
      average += m_dts[i];
    average /= DT_AVERAGE_COUNT;

    // 2: Current block
    glm::ivec3   position = glm::floor(world_data.player.transform.position);
    const Block* block    = world_data.get_block(position);

    int width, height;
    application.glfw_get_framebuffer_size(width, height);
    glm::vec2 viewport = glm::vec2(width, height);

    size_t n = 0;

    render_line(viewport, n++, fmt::format("position: x = {}, y = {}, z = {}", world_data.player.transform.position.x, world_data.player.transform.position.y, world_data.player.transform.position.z));
    render_line(viewport, n++, fmt::format("velocity: x = {}, y = {}, z = {}", world_data.player.velocity.x, world_data.player.velocity.y, world_data.player.velocity.z));
    render_line(viewport, n++, fmt::format("collided = {}", world_data.player.collided));
    render_line(viewport, n++, fmt::format("grounded = {}", world_data.player.grounded));
    render_line(viewport, n++, fmt::format("average frame time = {}", average));

    if(block)
      render_line(viewport, n++, fmt::format("block: position = {}, {}, {}, id = {}, sky = {}, light level = {}", position.x, position.y, position.z, block->id, block->sky, block->light_level));
    else
      render_line(viewport, n++, fmt::format("block: position = {}, {}, {}, not yet generated", position.x, position.y, position.z));

    if(world_data.selection)
      render_line(viewport, n++, fmt::format("selection: position = {}, {}, {}", world_data.selection->x, world_data.selection->y, world_data.selection->z));
    else
      render_line(viewport, n++, "selection: none");

    if(world_data.placement)
      render_line(viewport, n++, fmt::format("placement: position = {}, {}, {}", world_data.placement->x, world_data.placement->y, world_data.placement->z));
    else
      render_line(viewport, n++, "placement: none");
  }

private:
  graphics::UIRenderer            m_ui_renderer;
  std::unique_ptr<graphics::Font> m_font;

  float m_dts[DT_AVERAGE_COUNT];
};

std::unique_ptr<System> create_debug_system()
{
  return std::make_unique<DebugSystem>();
}


