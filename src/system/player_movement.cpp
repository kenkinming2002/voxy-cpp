#include <system/player_movement.hpp>

#include <world.hpp>

static constexpr float ROTATION_SPEED = 0.1f;

class PlayerMovementSystemImpl : public PlayerMovementSystem
{
private:
  void handle_event(World& world, SDL_Event event) override
  {
    switch(event.type)
    {
      case SDL_MOUSEMOTION:
        world.player().transform = world.player().transform.rotate(glm::vec3(0.0f,
          -event.motion.yrel * ROTATION_SPEED,
          -event.motion.xrel * ROTATION_SPEED
        ));
        break;
      case SDL_MOUSEWHEEL:
        world.camera().zoom(-event.wheel.y);
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
      translation = world.player().transform.gocal_to_global(translation);
      translation = glm::normalize(translation);
      translation *= dt;
      world.player().velocity += translation * 10.0f;
    }
  }
};

std::unique_ptr<PlayerMovementSystem> PlayerMovementSystem::create()
{
  return std::make_unique<PlayerMovementSystemImpl>();
}

