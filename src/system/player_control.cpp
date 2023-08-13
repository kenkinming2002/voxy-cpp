#include <system/player_control.hpp>

#include <application.hpp>
#include <world.hpp>
#include <directions.hpp>

#include <ray_cast.hpp>

#include <GLFW/glfw3.h>

#include <spdlog/spdlog.h>

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
  void on_update(Application& application, World& world, float dt) override
  {
    // 1: Jump
    if(application.glfw_get_key(GLFW_KEY_SPACE) == GLFW_PRESS)
      if(world.player.grounded)
      {
        world.player.grounded = false;
        world.player.apply_impulse(glm::vec3(0.0f, 0.0f, 8.0f));
      }

    // 2: Movement
    glm::vec3 translation = glm::vec3(0.0f);
    if(application.glfw_get_key(GLFW_KEY_D) == GLFW_PRESS) translation += world.player.transform.local_right();
    if(application.glfw_get_key(GLFW_KEY_A) == GLFW_PRESS) translation -= world.player.transform.local_right();
    if(application.glfw_get_key(GLFW_KEY_W) == GLFW_PRESS) translation += world.player.transform.local_forward();
    if(application.glfw_get_key(GLFW_KEY_S) == GLFW_PRESS) translation -= world.player.transform.local_forward();
    translation.z = 0.0f;
    if(glm::length(translation) != 0.0f)
      world.player.apply_force(5.0f * glm::normalize(translation), dt);

    // 3: Rotation
    double new_cursor_xpos;
    double new_cursor_ypos;
    application.glfw_get_cursor_pos(new_cursor_xpos, new_cursor_ypos);
    if(!m_first)
    {
      double xrel = new_cursor_xpos - m_cursor_xpos;
      double yrel = new_cursor_ypos - m_cursor_ypos;
      world.player.transform = world.player.transform.rotate(glm::vec3(0.0f,
        -yrel * ROTATION_SPEED,
        -xrel * ROTATION_SPEED
      ));
    }
    m_first = false;
    m_cursor_xpos = new_cursor_xpos;
    m_cursor_ypos = new_cursor_ypos;

    // 4: Block placement/destruction
    world.selection.reset();
    world.placement.reset();
    ray_cast(world.camera.transform.position, world.camera.transform.local_forward(), RAY_CAST_LENGTH, [&](glm::ivec3 block_position) -> bool {
        const Block *block = world.get_block(block_position);
        if(block && block->id != Block::ID_NONE)
          world.selection = block_position;
        else
          world.placement = block_position;
        return block && block->id != Block::ID_NONE;
    });

    // Can only place against a selected block
    if(!world.selection)
      world.placement.reset();

    if(application.glfw_get_mouse_button(GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
      if(world.selection)
        if(Block *block = world.get_block(*world.selection))
          if(block->id != Block::ID_NONE)
          {
            block->id = Block::ID_NONE;
            world.invalidate_mesh_major(*world.selection);
            world.invalidate_light(*world.selection);
            for(glm::ivec3 direction : DIRECTIONS)
            {
              glm::ivec3 neighbour_position = *world.selection + direction;
              world.invalidate_mesh_major(neighbour_position);
            }
          }

    if(application.glfw_get_mouse_button(GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
      if(world.placement)
        if(Block *block = world.get_block(*world.placement))
          if(block->id == Block::ID_NONE)
            if(!aabb_collide(world.player.transform.position, world.player.bounding_box, *world.placement, glm::vec3(1.0f, 1.0f, 1.0f))) // Cannot place a block that collide with the player
            {

              block->id = Block::ID_STONE;
              world.invalidate_mesh_major(*world.placement);
              world.invalidate_light(*world.placement);
              for(glm::ivec3 direction : DIRECTIONS)
              {
                glm::ivec3 neighbour_position = *world.placement + direction;
                world.invalidate_mesh_major(neighbour_position);
              }
            }
  }

private:
  bool   m_first = true;
  double m_cursor_xpos;
  double m_cursor_ypos;
};

std::unique_ptr<System> create_player_control_system()
{
  return std::make_unique<PlayerControlSystem>();
}

