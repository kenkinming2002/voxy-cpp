#include <player_controller.hpp>

#include <directions.hpp>
#include <ray_cast.hpp>

PlayerController::PlayerController()
{
  m_first    = true;
  m_cooldown = 0.0f;
}

static bool aabb_collide(glm::vec3 position1, glm::vec3 dimension1, glm::vec3 position2, glm::vec3 dimension2)
{
  glm::vec3 min = glm::max(position1,            position2);
  glm::vec3 max = glm::min(position1+dimension1, position2+dimension2);
  for(int i=0; i<3; ++i)
    if(min[i] >= max[i])
      return false;
  return true;
}

void PlayerController::update(Application& application, World& world, LightManager& light_manager, float dt)
{
  Entity& player_entity = world.dimension.entities.at(world.player.entity_id);

  // 1: Jump
  if(application.glfw_get_key(GLFW_KEY_SPACE) == GLFW_PRESS)
    if(player_entity.grounded)
    {
      player_entity.grounded = false;
      entity_apply_impulse(player_entity, JUMP_STRENGTH * glm::vec3(0.0f, 0.0f, 1.0f));
    }

  // 2: Movement
  glm::vec3 translation = glm::vec3(0.0f);
  if(application.glfw_get_key(GLFW_KEY_D) == GLFW_PRESS) translation += player_entity.transform.local_right();
  if(application.glfw_get_key(GLFW_KEY_A) == GLFW_PRESS) translation -= player_entity.transform.local_right();
  if(application.glfw_get_key(GLFW_KEY_W) == GLFW_PRESS) translation += player_entity.transform.local_forward();
  if(application.glfw_get_key(GLFW_KEY_S) == GLFW_PRESS) translation -= player_entity.transform.local_forward();

  if(glm::vec3 direction = translation; direction.z = 0.0f, glm::length(direction) != 0.0f)
    entity_apply_force(player_entity, MOVEMENT_SPEED * glm::normalize(direction), dt);
  else if(glm::vec3 direction = -player_entity.velocity; direction.z = 0.0f, glm::length(direction) != 0.0f)
    entity_apply_force(player_entity, MOVEMENT_SPEED * glm::normalize(direction), dt, glm::length(direction));

  // 3: Rotation
  double new_cursor_xpos;
  double new_cursor_ypos;
  application.glfw_get_cursor_pos(new_cursor_xpos, new_cursor_ypos);
  if(!m_first)
  {
    double xrel = new_cursor_xpos - m_cursor_xpos;
    double yrel = new_cursor_ypos - m_cursor_ypos;
    player_entity.transform = player_entity.transform.rotate(glm::vec3(0.0f,
      -yrel * ROTATION_SPEED,
      -xrel * ROTATION_SPEED
    ));
  }
  m_first = false;
  m_cursor_xpos = new_cursor_xpos;
  m_cursor_ypos = new_cursor_ypos;

  // 4: Block placement/destruction
  m_cooldown = std::max(m_cooldown - dt, 0.0f);

  world.player.selection.reset();
  world.player.placement.reset();
  ray_cast(player_entity.transform.position + glm::vec3(0.0f, 0.0f, player_entity.eye), player_entity.transform.local_forward(), RAY_CAST_LENGTH, [&](glm::ivec3 block_position) -> bool {
      const Block *block = get_block(world, block_position);
      if(block && block->id != BLOCK_ID_NONE)
        world.player.selection = block_position;
      else
        world.player.placement = block_position;
      return block && block->id != BLOCK_ID_NONE;
  });

  // Can only place against a selected block
  if(!world.player.selection)
    world.player.placement.reset();

  if(m_cooldown == 0.0f)
    if(application.glfw_get_mouse_button(GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
      if(world.player.selection)
        if(Block *block = get_block(world, *world.player.selection))
          if(block->id != BLOCK_ID_NONE)
          {
            if(block->destroy_level != 15)
              ++block->destroy_level;
            else
              block->id = BLOCK_ID_NONE;

            invalidate_mesh(world, *world.player.selection);
            light_manager.invalidate(*world.player.selection);
            for(glm::ivec3 direction : DIRECTIONS)
            {
              glm::ivec3 neighbour_position = *world.player.selection + direction;
              invalidate_mesh(world, neighbour_position);
            }
            m_cooldown = ACTION_COOLDOWN;
          }

  if(m_cooldown == 0.0f)
    if(application.glfw_get_mouse_button(GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
      if(world.player.placement)
        if(Block *block = get_block(world, *world.player.placement))
          if(block->id == BLOCK_ID_NONE)
            if(!aabb_collide(player_entity.transform.position, player_entity.dimension, *world.player.placement, glm::vec3(1.0f, 1.0f, 1.0f))) // Cannot place a block that collide with the player
            {
              block->id = BLOCK_ID_STONE;
              invalidate_mesh(world, *world.player.placement);
              light_manager.invalidate(*world.player.placement);
              for(glm::ivec3 direction : DIRECTIONS)
              {
                glm::ivec3 neighbour_position = *world.player.placement + direction;
                invalidate_mesh(world, neighbour_position);
              }
              m_cooldown = ACTION_COOLDOWN;
            }
}

void PlayerController::render(const Camera& camera, const World& world, graphics::WireframeRenderer& wireframe_renderer)
{
  if(world.player.selection) wireframe_renderer.render_cube(camera, *world.player.selection, glm::vec3(1.0f), glm::vec3(0.6f, 0.6f, 0.6f), UI_SELECTION_THICKNESS);
  if(world.player.placement) wireframe_renderer.render_cube(camera, *world.player.placement, glm::vec3(1.0f), glm::vec3(0.6f, 0.6f, 0.6f), UI_SELECTION_THICKNESS);
}

