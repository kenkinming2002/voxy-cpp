#include <system/player_control.hpp>

#include <types/world.hpp>
#include <types/directions.hpp>

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


class PlayerControlSystemImpl : public PlayerControlSystem
{
private:
  void handle_event(World& world, SDL_Event event) override
  {
    switch(event.type)
    {
      case SDL_MOUSEBUTTONDOWN:
        switch(event.button.button)
        {
          case SDL_BUTTON_LEFT:
            if(world.selection)
              if(Block *block = world.dimension.get_block(*world.selection))
                if(block->presence)
                {
                  block->presence = false;
                  world.dimension.major_invalidate_mesh(*world.selection);
                  world.dimension.lighting_invalidate(*world.selection);
                  for(glm::ivec3 direction : DIRECTIONS)
                  {
                    glm::ivec3 neighbour_position = *world.selection + direction;
                    world.dimension.major_invalidate_mesh(neighbour_position);
                  }
                }
            break;
          case SDL_BUTTON_RIGHT:
            if(world.placement)
              if(Block *block = world.dimension.get_block(*world.placement))
                if(!block->presence)
                {
                  // Cannot place a block that collide with the player
                  if(aabb_collide(world.player.transform.position, world.player.bounding_box, *world.placement, glm::vec3(1.0f, 1.0f, 1.0f)))
                    break;

                  block->presence = true;
                  block->id       = Block::ID_STONE;

                  world.dimension.major_invalidate_mesh(*world.placement);
                  world.dimension.lighting_invalidate(*world.placement);
                  for(glm::ivec3 direction : DIRECTIONS)
                  {
                    glm::ivec3 neighbour_position = *world.placement + direction;
                    world.dimension.major_invalidate_mesh(neighbour_position);
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

  void update(World& world, float dt) override
  {
    glm::vec3 translation = glm::vec3(0.0f);

    const Uint8 *keys = SDL_GetKeyboardState(nullptr);
    if(keys[SDL_SCANCODE_SPACE])  translation.z += 1.0f;
    if(keys[SDL_SCANCODE_LSHIFT]) translation.z -= 1.0f;
    if(keys[SDL_SCANCODE_W])      translation.y += 1.0f;
    if(keys[SDL_SCANCODE_S])      translation.y -= 1.0f;
    if(keys[SDL_SCANCODE_D])      translation.x += 1.0f;
    if(keys[SDL_SCANCODE_A])      translation.x -= 1.0f;
    if(glm::length(translation) != 0.0f)
    {
      translation = world.player.transform.gocal_to_global(translation);
      translation = glm::normalize(translation);
      translation *= dt;
      world.player.velocity += translation * 10.0f;
    }

    world.selection.reset();
    world.placement.reset();
    ray_cast(world.camera.transform.position, world.camera.transform.local_forward(), RAY_CAST_LENGTH, [&](glm::ivec3 block_position) -> bool {
        const Block *block = world.dimension.get_block(block_position);
        if(block && block->presence)
          world.selection = block_position;
        else
          world.placement = block_position;
        return block && block->presence;
    });

    // Can only place against a selected block
    if(!world.selection)
      world.placement.reset();
  }
};

std::unique_ptr<PlayerControlSystem> PlayerControlSystem::create()
{
  return std::make_unique<PlayerControlSystemImpl>();
}
