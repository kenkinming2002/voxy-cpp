#include <debug_renderer.hpp>

#include <fmt/format.h>

DebugRenderer::DebugRenderer()
{
  m_font = std::make_unique<graphics::Font>(DEBUG_FONT, DEBUG_FONT_HEIGHT);
  for(size_t i=0; i<DT_AVERAGE_COUNT; ++i)
    m_dts[i] = 0.0f;
}

void DebugRenderer::render(Application& application, const World& world)
{
    // 1: Frame time
    float average = 0.0f;
    for(size_t i=0; i<DT_AVERAGE_COUNT; ++i)
      average += m_dts[i];
    average /= DT_AVERAGE_COUNT;

    // 2: Current block
    glm::ivec3   position = glm::floor(world.player.entity.transform.position);
    const Block* block    = get_block(world, position);

    int width, height;
    application.glfw_get_framebuffer_size(width, height);
    glm::vec2 viewport = glm::vec2(width, height);

    size_t n = 0;

    render_line(viewport, n++, fmt::format("position: x = {}, y = {}, z = {}", world.player.entity.transform.position.x, world.player.entity.transform.position.y, world.player.entity.transform.position.z));
    render_line(viewport, n++, fmt::format("velocity: x = {}, y = {}, z = {}", world.player.entity.velocity.x, world.player.entity.velocity.y, world.player.entity.velocity.z));
    render_line(viewport, n++, fmt::format("collided = {}", world.player.entity.collided));
    render_line(viewport, n++, fmt::format("grounded = {}", world.player.entity.grounded));
    render_line(viewport, n++, fmt::format("average frame time = {}", average));

    if(block)
      render_line(viewport, n++, fmt::format("block: position = {}, {}, {}, id = {}, sky = {}, light level = {}", position.x, position.y, position.z, block->id, block->sky, block->light_level));
    else
      render_line(viewport, n++, fmt::format("block: position = {}, {}, {}, not yet generated", position.x, position.y, position.z));

    if(world.player.selection)
      render_line(viewport, n++, fmt::format("player.selection: position = {}, {}, {}", world.player.selection->x, world.player.selection->y, world.player.selection->z));
    else
      render_line(viewport, n++, "selection: none");

    if(world.player.placement)
      render_line(viewport, n++, fmt::format("player.placement: position = {}, {}, {}", world.player.placement->x, world.player.placement->y, world.player.placement->z));
    else
      render_line(viewport, n++, "player.placement: none");
}

void DebugRenderer::render_line(glm::vec2 viewport, size_t n, const std::string& line)
{
  glm::vec2 position = DEBUG_MARGIN + glm::vec2(0.0f, n * DEBUG_FONT_HEIGHT);
  m_font->render(m_ui_renderer, viewport, position, line.c_str());
}
