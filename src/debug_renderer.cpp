#include <debug_renderer.hpp>

#include <fmt/format.h>

DebugRenderer::DebugRenderer()
{
  m_font = std::make_unique<graphics::Font>(DEBUG_FONT, DEBUG_FONT_HEIGHT);
  for(size_t i=0; i<DT_AVERAGE_COUNT; ++i)
    m_dts[i] = 0.0f;
}

void DebugRenderer::update(float dt)
{
  for(size_t i=0; i<DT_AVERAGE_COUNT-1; ++i)
    m_dts[i] = m_dts[i+1];
  m_dts[DT_AVERAGE_COUNT-1] = dt;
}

void DebugRenderer::render(glm::vec2 viewport, const World& world, graphics::UIRenderer& ui_renderer)
{
  // 1: Frame time
  float average = 0.0f;
  for(size_t i=0; i<DT_AVERAGE_COUNT; ++i)
    average += m_dts[i];
  average /= DT_AVERAGE_COUNT;

  // 2: Current block
  const Player& player        = world.players.front();
  const Entity& player_entity = world.entities.at(player.entity_id);
  glm::ivec3   position = glm::floor(player_entity.transform.position);
  const Block* block    = get_block(world, position);

  size_t n = 0;

  render_line(viewport, n++, fmt::format("position: x = {}, y = {}, z = {}", player_entity.transform.position.x, player_entity.transform.position.y, player_entity.transform.position.z), ui_renderer);
  render_line(viewport, n++, fmt::format("velocity: x = {}, y = {}, z = {}", player_entity.velocity.x, player_entity.velocity.y, player_entity.velocity.z), ui_renderer);
  render_line(viewport, n++, fmt::format("collided = {}", player_entity.collided), ui_renderer);
  render_line(viewport, n++, fmt::format("grounded = {}", player_entity.grounded), ui_renderer);
  render_line(viewport, n++, fmt::format("average update time = {}", average), ui_renderer);

  if(block)
    render_line(viewport, n++, fmt::format("block: position = {}, {}, {}, id = {}, sky = {}, light level = {}", position.x, position.y, position.z, block->id, block->sky, block->light_level), ui_renderer);
  else
    render_line(viewport, n++, fmt::format("block: position = {}, {}, {}, not yet generated", position.x, position.y, position.z), ui_renderer);

  if(player.selection)
    render_line(viewport, n++, fmt::format("player.selection: position = {}, {}, {}", player.selection->x, player.selection->y, player.selection->z), ui_renderer);
  else
    render_line(viewport, n++, "selection: none", ui_renderer);

  if(player.placement)
    render_line(viewport, n++, fmt::format("player.placement: position = {}, {}, {}", player.placement->x, player.placement->y, player.placement->z), ui_renderer);
  else
    render_line(viewport, n++, "player.placement: none", ui_renderer);
}

void DebugRenderer::render_line(glm::vec2 viewport, size_t n, const std::string& line, graphics::UIRenderer& ui_renderer)
{
  glm::vec2 position = DEBUG_MARGIN + glm::vec2(0.0f, n * DEBUG_FONT_HEIGHT);
  m_font->render(ui_renderer, viewport, position, line.c_str());
}
