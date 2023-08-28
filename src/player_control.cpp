#include <player_control.hpp>

#include <directions.hpp>
#include <ray_cast.hpp>

static constexpr float ROTATION_SPEED = 0.1f;
static constexpr float MOVEMENT_SPEED = 5.0f;

static constexpr float JUMP_STRENGTH = 5.0f;

static constexpr float RAY_CAST_LENGTH = 20.0f;
static constexpr float ACTION_COOLDOWN = 0.1f;

static bool aabb_collide(glm::vec3 position1, glm::vec3 dimension1, glm::vec3 position2, glm::vec3 dimension2)
{
  glm::vec3 min = glm::max(position1,            position2);
  glm::vec3 max = glm::min(position1+dimension1, position2+dimension2);
  for(int i=0; i<3; ++i)
    if(min[i] >= max[i])
      return false;
  return true;
}

void update_player_control(World& world, LightManager& light_manager, float dt)
{
  for(Player& player : world.players)
  {
    Entity& player_entity = world.entities.at(player.entity_id);

    // 1: Jump
    if(player.key_space)
      if(player_entity.grounded)
      {
        player_entity.grounded = false;
        entity_apply_impulse(player_entity, JUMP_STRENGTH * glm::vec3(0.0f, 0.0f, 1.0f));
      }

    // 2: Movement
    glm::vec3 translation = glm::vec3(0.0f);
    if(player.key_d) translation += player_entity.transform.local_right();
    if(player.key_a) translation -= player_entity.transform.local_right();
    if(player.key_w) translation += player_entity.transform.local_forward();
    if(player.key_s) translation -= player_entity.transform.local_forward();

    if(glm::vec3 direction = translation; direction.z = 0.0f, glm::length(direction) != 0.0f)
      entity_apply_force(player_entity, MOVEMENT_SPEED * glm::normalize(direction), dt);
    else if(glm::vec3 direction = -player_entity.velocity; direction.z = 0.0f, glm::length(direction) != 0.0f)
      entity_apply_force(player_entity, MOVEMENT_SPEED * glm::normalize(direction), dt, glm::length(direction));

    // 3: Rotation
    player_entity.transform = player_entity.transform.rotate(glm::vec3(0.0f,
      -player.cursor_motion_y * ROTATION_SPEED,
      -player.cursor_motion_x * ROTATION_SPEED
    ));

    // 4: Block placement/destruction
    player.cooldown = std::max(player.cooldown - dt, 0.0f);

    RayCastBlocksResult ray_cast_result = ray_cast_blocks(world, player_entity.transform.position + glm::vec3(0.0f, 0.0f, player_entity.eye), player_entity.transform.local_forward(), RAY_CAST_LENGTH);
    switch(ray_cast_result.type)
    {
    case RayCastBlocksResult::Type::INSIDE_BLOCK:
      player.selection = ray_cast_result.position;
      player.placement = std::nullopt;
      break;
    case RayCastBlocksResult::Type::HIT:
      player.selection = ray_cast_result.position;
      player.placement = ray_cast_result.position + ray_cast_result.normal;
      break;
    case RayCastBlocksResult::Type::NONE:
      player.selection = std::nullopt;
      player.placement = std::nullopt;
      break;
    }

    if(player.cooldown == 0.0f)
      if(player.mouse_button_left)
        if(player.selection)
          if(Block *block = get_block(world, *player.selection))
            if(block->id != BLOCK_ID_NONE)
            {
              if(block->destroy_level != 15)
                ++block->destroy_level;
              else
                block->id = BLOCK_ID_NONE;

              invalidate_mesh(world, *player.selection);
              light_manager.invalidate(*player.selection);
              for(glm::ivec3 direction : DIRECTIONS)
              {
                glm::ivec3 neighbour_position = *player.selection + direction;
                invalidate_mesh(world, neighbour_position);
              }
              player.cooldown = ACTION_COOLDOWN;
            }

    if(player.cooldown == 0.0f)
      if(player.mouse_button_right)
        if(player.placement)
          if(Block *block = get_block(world, *player.placement))
            if(block->id == BLOCK_ID_NONE)
              if(!aabb_collide(player_entity.transform.position, player_entity.dimension, *player.placement, glm::vec3(1.0f, 1.0f, 1.0f))) // Cannot place a block that collide with the player
              {
                block->id = BLOCK_ID_STONE;
                invalidate_mesh(world, *player.placement);
                light_manager.invalidate(*player.placement);
                for(glm::ivec3 direction : DIRECTIONS)
                {
                  glm::ivec3 neighbour_position = *player.placement + direction;
                  invalidate_mesh(world, neighbour_position);
                }
                player.cooldown = ACTION_COOLDOWN;
              }
  }
}

