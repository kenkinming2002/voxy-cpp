#include <system/camera_follow.hpp>

#include <world.hpp>

class CameraFollowSystemImpl : public CameraFollowSystem
{
private:
  void update(World& world, float dt) override
  {
    world.camera().transform           = world.player().transform;
    world.camera().transform.position += glm::vec3(0.5f, 0.5f, 1.5f);
  }
};

std::unique_ptr<CameraFollowSystem> CameraFollowSystem::create()
{
  return std::make_unique<CameraFollowSystemImpl>();
}

