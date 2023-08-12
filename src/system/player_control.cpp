#include <system/player_control.hpp>

#include <world.hpp>
#include <directions.hpp>

#include <ray_cast.hpp>

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
  void on_event(World& world, SDL_Event event) override
  {
    switch(event.type)
    {
      case SDL_MOUSEBUTTONDOWN:
        switch(event.button.button)
        {
          case SDL_BUTTON_LEFT:
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
            break;
          case SDL_BUTTON_RIGHT:
            if(world.placement)
              if(Block *block = world.get_block(*world.placement))
                if(block->id == Block::ID_NONE)
                {
                  // Cannot place a block that collide with the player
                  if(aabb_collide(world.player.transform.position, world.player.bounding_box, *world.placement, glm::vec3(1.0f, 1.0f, 1.0f)))
                    break;

                  block->id = Block::ID_STONE;
                  world.invalidate_mesh_major(*world.placement);
                  world.invalidate_light(*world.placement);
                  for(glm::ivec3 direction : DIRECTIONS)
                  {
                    glm::ivec3 neighbour_position = *world.placement + direction;
                    world.invalidate_mesh_major(neighbour_position);
                  }
                }
            break;
        }
        break;
      case SDL_MOUSEMOTION:
        world.player.transform = world.player.transform.rotate(glm::vec3(0.0f,
          -event.motion.yrel * ROTATION_SPEED,
          -event.motion.xrel * ROTATION_SPEED
        ));
        break;
      case SDL_MOUSEWHEEL:
        world.camera.zoom(-event.wheel.y);
        break;
    }
  }

  void on_update(World& world, float dt) override
  {
    const Uint8 *keys = SDL_GetKeyboardState(nullptr);

    // 1: Jump
    if(keys[SDL_SCANCODE_SPACE])
      if(world.player.grounded)
      {
        world.player.grounded = false;
        world.player.apply_impulse(glm::vec3(0.0f, 0.0f, 8.0f));
      }

    // 2: Movement
    glm::vec3 translation = glm::vec3(0.0f);
    if(keys[SDL_SCANCODE_D]) translation += world.player.transform.local_right();
    if(keys[SDL_SCANCODE_A]) translation -= world.player.transform.local_right();
    if(keys[SDL_SCANCODE_W]) translation += world.player.transform.local_forward();
    if(keys[SDL_SCANCODE_S]) translation -= world.player.transform.local_forward();
    translation.z = 0.0f;
    if(glm::length(translation) != 0.0f)
      world.player.apply_force(5.0f * glm::normalize(translation), dt);

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
  }
};

std::unique_ptr<System> create_player_control_system()
{
  return std::make_unique<PlayerControlSystem>();
}

