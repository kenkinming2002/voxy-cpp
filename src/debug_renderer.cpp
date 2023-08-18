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
    glm::ivec3   position = glm::floor(world.player.transform.position);
    const Block* block    = get_block(world, position);

    int width, height;
    application.glfw_get_framebuffer_size(width, height);
    glm::vec2 viewport = glm::vec2(width, height);

    size_t n = 0;

    render_line(viewport, n++, fmt::format("position: x = {}, y = {}, z = {}", world.player.transform.position.x, world.player.transform.position.y, world.player.transform.position.z));
    render_line(viewport, n++, fmt::format("velocity: x = {}, y = {}, z = {}", world.player.velocity.x, world.player.velocity.y, world.player.velocity.z));
    render_line(viewport, n++, fmt::format("collided = {}", world.player.collided));
    render_line(viewport, n++, fmt::format("grounded = {}", world.player.grounded));
    render_line(viewport, n++, fmt::format("average frame time = {}", average));

    if(block)
      render_line(viewport, n++, fmt::format("block: position = {}, {}, {}, id = {}, sky = {}, light level = {}", position.x, position.y, position.z, block->id, block->sky, block->light_level));
    else
      render_line(viewport, n++, fmt::format("block: position = {}, {}, {}, not yet generated", position.x, position.y, position.z));

    if(world.selection)
      render_line(viewport, n++, fmt::format("selection: position = {}, {}, {}", world.selection->x, world.selection->y, world.selection->z));
    else
      render_line(viewport, n++, "selection: none");

    if(world.placement)
      render_line(viewport, n++, fmt::format("placement: position = {}, {}, {}", world.placement->x, world.placement->y, world.placement->z));
    else
      render_line(viewport, n++, "placement: none");
}

void DebugRenderer::render_line(glm::vec2 viewport, size_t n, const std::string& line)
{
  glm::vec2 position = DEBUG_MARGIN + glm::vec2(0.0f, n * DEBUG_FONT_HEIGHT);
  m_font->render(m_ui_renderer, viewport, position, line.c_str());
}
