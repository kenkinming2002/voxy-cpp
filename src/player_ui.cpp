#include <player_ui.hpp>

static constexpr float UI_SELECTION_THICKNESS = 3.0f;

void render_player_ui(const graphics::Camera& camera, const World& world, graphics::WireframeRenderer& wireframe_renderer)
{
  const Player& player = world.players.front();
  if(player.selection) wireframe_renderer.render_cube(camera, *player.selection, glm::vec3(1.0f), glm::vec3(0.6f, 0.6f, 0.6f), UI_SELECTION_THICKNESS);
  if(player.placement) wireframe_renderer.render_cube(camera, *player.placement, glm::vec3(1.0f), glm::vec3(0.6f, 0.6f, 0.6f), UI_SELECTION_THICKNESS);
}

