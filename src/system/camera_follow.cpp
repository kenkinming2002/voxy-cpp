#include <system/camera_follow.hpp>

#include <application.hpp>
#include <world.hpp>

class CameraFollowSystem : public System
{
private:
  void on_update(Application& application, const WorldConfig& world_config, WorldData& world_data, float dt) override
  {
    world_data.camera.transform           = world_data.player.transform;
    world_data.camera.transform.position += glm::vec3(0.5f, 0.5f, 1.5f);

    int width, height;
    application.glfw_get_framebuffer_size(width, height);
    glViewport(0, 0, width, height);
    world_data.camera.aspect = (double)width / (double)height;
  }
};

std::unique_ptr<System> create_camera_follow_system()
{
  return std::make_unique<CameraFollowSystem>();
}

