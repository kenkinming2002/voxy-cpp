#include <system/player_control.hpp>

#include <application.hpp>
#include <world.hpp>
#include <directions.hpp>

#include <ray_cast.hpp>

#include <GLFW/glfw3.h>

#include <spdlog/spdlog.h>

#include <algorithm>

static constexpr float ROTATION_SPEED = 0.1f;
static constexpr float RAY_CAST_LENGTH = 20.0f;

static bool aabb_collide(glm::vec3 position1, glm::vec3 dimension1, glm::vec3 position2, glm::vec3 dimension2)
{
  glm::vec3 min = glm::max(position1,            position2);
  glm::vec3 max = glm::min(position1+dimension1, position2+dimension2);
  for(int i=0; i<3; ++i)
    if(min[i] >= max[i])
      return false;
  return true;
}

class PlayerControlSystem : public System
{
private:
  static constexpr float ACTION_COOLDOWN = 0.1f;

private:
  void on_update(Application& application, const WorldConfig& world_config, World& world_data, float dt) override
  {
    // 1: Jump
    if(application.glfw_get_key(GLFW_KEY_SPACE) == GLFW_PRESS)
      if(world_data.player.grounded)
      {
        world_data.player.grounded = false;
        entity_apply_impulse(world_data.player, glm::vec3(0.0f, 0.0f, 8.0f));
      }

    // 2: Movement
    glm::vec3 translation = glm::vec3(0.0f);
    if(application.glfw_get_key(GLFW_KEY_D) == GLFW_PRESS) translation += world_data.player.transform.local_right();
    if(application.glfw_get_key(GLFW_KEY_A) == GLFW_PRESS) translation -= world_data.player.transform.local_right();
    if(application.glfw_get_key(GLFW_KEY_W) == GLFW_PRESS) translation += world_data.player.transform.local_forward();
    if(application.glfw_get_key(GLFW_KEY_S) == GLFW_PRESS) translation -= world_data.player.transform.local_forward();
    translation.z = 0.0f;
    if(glm::length(translation) != 0.0f)
      entity_apply_force(world_data.player, 5.0f * glm::normalize(translation), dt);

    // 3: Rotation
    double new_cursor_xpos;
    double new_cursor_ypos;
    application.glfw_get_cursor_pos(new_cursor_xpos, new_cursor_ypos);
    if(!m_first)
    {
      double xrel = new_cursor_xpos - m_cursor_xpos;
      double yrel = new_cursor_ypos - m_cursor_ypos;
      world_data.player.transform = world_data.player.transform.rotate(glm::vec3(0.0f,
        -yrel * ROTATION_SPEED,
        -xrel * ROTATION_SPEED
      ));
    }
    m_first = false;
    m_cursor_xpos = new_cursor_xpos;
    m_cursor_ypos = new_cursor_ypos;

    // 4: Block placement/destruction
    m_cooldown = std::max(m_cooldown - dt, 0.0f);

    world_data.selection.reset();
    world_data.placement.reset();
    ray_cast(world_data.player.transform.position + world_data.player.eye_offset, world_data.player.transform.local_forward(), RAY_CAST_LENGTH, [&](glm::ivec3 block_position) -> bool {
        const Block *block = get_block(world_data, block_position);
        if(block && block->id != BLOCK_ID_NONE)
          world_data.selection = block_position;
        else
          world_data.placement = block_position;
        return block && block->id != BLOCK_ID_NONE;
    });

    // Can only place against a selected block
    if(!world_data.selection)
      world_data.placement.reset();

    if(m_cooldown == 0.0f)
      if(application.glfw_get_mouse_button(GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        if(world_data.selection)
          if(Block *block = get_block(world_data, *world_data.selection))
            if(block->id != BLOCK_ID_NONE)
            {
              if(block->destroy_level != 15)
                ++block->destroy_level;
              else
                block->id = BLOCK_ID_NONE;

              invalidate_mesh_major(world_data, *world_data.selection);
              invalidate_light     (world_data, *world_data.selection);
              for(glm::ivec3 direction : DIRECTIONS)
              {
                glm::ivec3 neighbour_position = *world_data.selection + direction;
                invalidate_mesh_major(world_data, neighbour_position);
              }
              m_cooldown = ACTION_COOLDOWN;
            }

    if(m_cooldown == 0.0f)
      if(application.glfw_get_mouse_button(GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
        if(world_data.placement)
          if(Block *block = get_block(world_data, *world_data.placement))
            if(block->id == BLOCK_ID_NONE)
              if(!aabb_collide(world_data.player.transform.position, world_data.player.bounding_box, *world_data.placement, glm::vec3(1.0f, 1.0f, 1.0f))) // Cannot place a block that collide with the player
              {
                block->id = BLOCK_ID_STONE;
                invalidate_mesh_major(world_data, *world_data.placement);
                invalidate_light     (world_data, *world_data.placement);
                for(glm::ivec3 direction : DIRECTIONS)
                {
                  glm::ivec3 neighbour_position = *world_data.placement + direction;
                  invalidate_mesh_major(world_data, neighbour_position);
                }
                m_cooldown = ACTION_COOLDOWN;
              }
  }

private:
  bool   m_first = true;
  double m_cursor_xpos;
  double m_cursor_ypos;

  float m_cooldown = 0.0f;
};

std::unique_ptr<System> create_player_control_system()
{
  return std::make_unique<PlayerControlSystem>();
}

