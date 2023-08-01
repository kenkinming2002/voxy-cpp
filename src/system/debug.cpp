#include <system/debug.hpp>

#include <types/world.hpp>

#include <text_renderer.hpp>

#include <fmt/format.h>

static constexpr glm::vec2   DEBUG_MARGIN       = glm::vec2(3.0f, 3.0f);
static constexpr const char *DEBUG_FONT        = "assets/arial.ttf";
static constexpr float       DEBUG_FONT_HEIGHT = 20.0f;

static constexpr size_t DT_AVERAGE_COUNT = 32;

class DebugSystemImpl : public DebugSystem
{
public:
  DebugSystemImpl() : m_text_renderer(DEBUG_FONT, DEBUG_FONT_HEIGHT)
  {
    for(size_t i=0; i<DT_AVERAGE_COUNT; ++i)
      m_dts[i] = 0.0f;
  }

private:
  void update(float dt) override
  {
    for(size_t i=0; i<DT_AVERAGE_COUNT-1; ++i)
      m_dts[i] = m_dts[i+1];
    m_dts[DT_AVERAGE_COUNT-1] = dt;
  }

  void render(const World& world) override
  {
    // 1: Frame time
    float average = 0.0f;
    for(size_t i=0; i<DT_AVERAGE_COUNT; ++i)
      average += m_dts[i];
    average /= DT_AVERAGE_COUNT;

    // 2: Current block
    glm::ivec3   position = glm::floor(world.player.transform.position);
    const Block* block    = world.dimension.get_block(position);

    std::string line;
    glm::vec2   cursor = DEBUG_MARGIN;

    line = fmt::format("position: x = {}, y = {}, z = {}", world.player.transform.position.x, world.player.transform.position.y, world.player.transform.position.z);
    m_text_renderer.render(cursor, line.c_str());
    cursor.x = DEBUG_MARGIN.x;
    cursor.y += DEBUG_FONT_HEIGHT;

    line = fmt::format("velocity: x = {}, y = {}, z = {}", world.player.velocity.x, world.player.velocity.y, world.player.velocity.z);
    m_text_renderer.render(cursor, line.c_str());
    cursor.x = DEBUG_MARGIN.x;
    cursor.y += DEBUG_FONT_HEIGHT;

    line = fmt::format("collided = {}", world.player.collided);
    m_text_renderer.render(cursor, line.c_str());
    cursor.x = DEBUG_MARGIN.x;
    cursor.y += DEBUG_FONT_HEIGHT;

    line = fmt::format("average frame time = {}", average);
    m_text_renderer.render(cursor, line.c_str());
    cursor.x = DEBUG_MARGIN.x;
    cursor.y += DEBUG_FONT_HEIGHT;

    if(block)
    {
      line = fmt::format("block: position = {}, {}, {}, presence = {}, id = {}, sky = {}, light level = {}", position.x, position.y, position.z, block->presence, block->id, block->sky, block->light_level);
      m_text_renderer.render(cursor, line.c_str());
      cursor.x = DEBUG_MARGIN.x;
      cursor.y += DEBUG_FONT_HEIGHT;
    }
    else
    {
      line = fmt::format("block: position = {}, {}, {}, not yet generated", position.x, position.y, position.z);
      m_text_renderer.render(cursor, line.c_str());
      cursor.x = DEBUG_MARGIN.x;
      cursor.y += DEBUG_FONT_HEIGHT;
    }
  }

private:
  TextRenderer m_text_renderer;
  float m_dts[DT_AVERAGE_COUNT];
};

std::unique_ptr<DebugSystem> DebugSystem::create()
{
  return std::make_unique<DebugSystemImpl>();
}


