#include <system/camera_follow.hpp>

#include <world.hpp>

class CameraFollowSystem : public System
{
private:
  void on_update(Application& application, World& world, float dt) override
  {
    world.camera.transform           = world.player.transform;
    world.camera.transform.position += glm::vec3(0.5f, 0.5f, 1.5f);
  }
};

std::unique_ptr<System> create_camera_follow_system()
{
  return std::make_unique<CameraFollowSystem>();
}

