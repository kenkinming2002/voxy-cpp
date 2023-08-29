#include <player_ui.hpp>

#include <ray_cast.hpp>

static constexpr float UI_SELECTION_THICKNESS = 3.0f;
static constexpr float RAY_CAST_LENGTH        = 20.0f;

void render_player_ui(const graphics::Camera& camera, const World& world, graphics::WireframeRenderer& wireframe_renderer)
{
  const Player& player        = world.players.front();
  const Entity& player_entity = world.entities.at(player.entity_id);

  RayCastBlocksResult ray_cast_result = ray_cast_blocks(world, player_entity.transform.position + glm::vec3(0.0f, 0.0f, player_entity.eye), player_entity.transform.local_forward(), RAY_CAST_LENGTH);

  std::optional<glm::ivec3> selection, placement;
  switch(ray_cast_result.type)
  {
    case RayCastBlocksResult::Type::INSIDE_BLOCK:
      selection = ray_cast_result.position;
      placement = std::nullopt;
      break;
    case RayCastBlocksResult::Type::HIT:
      selection = ray_cast_result.position;
      placement = ray_cast_result.position + ray_cast_result.normal;
      break;
    case RayCastBlocksResult::Type::NONE:
      selection = std::nullopt;
      placement = std::nullopt;
      break;
  }

  if(selection) wireframe_renderer.render_cube(camera, *selection, glm::vec3(1.0f), glm::vec3(0.6f, 0.6f, 0.6f), UI_SELECTION_THICKNESS);
  if(placement) wireframe_renderer.render_cube(camera, *placement, glm::vec3(1.0f), glm::vec3(0.6f, 0.6f, 0.6f), UI_SELECTION_THICKNESS);
}

