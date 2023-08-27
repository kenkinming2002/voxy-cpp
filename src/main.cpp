#include <world.hpp>

#include <world_generator.hpp>
#include <light_manager.hpp>
#include <physics.hpp>
#include <player_controller.hpp>

#include <graphics/window.hpp>
#include <graphics/wireframe_renderer.hpp>

#include <camera.hpp>
#include <timer.hpp>

#include <world_renderer.hpp>
#include <debug_renderer.hpp>

#include <resource_pack.hpp>

int main()
{
  static constexpr float FIXED_DT = 1.0f / 20.0f;

  World world = load_world("world");

  WorldGenerator   world_generator(load_world_generation_config("world"));
  PlayerController player_controller;
  LightManager     light_manager;

  graphics::Window            window("voxy", 1024, 720);
  graphics::WireframeRenderer wireframer_renderer;

  WorldRenderer world_renderer(load_resource_pack("resource_pack"));
  DebugRenderer debug_renderer;

  bool third_person = false;
  window.glfw_on_key([&third_person](int key, int scancode, int action, int mods) {
    if(key == GLFW_KEY_F5 && action == GLFW_PRESS)
      third_person = !third_person;
  });

  Timer timer;
  for(;;)
  {
    window.poll_events();
    if(window.should_close())
      return 0;

    // 1: Update
    if(timer.tick(FIXED_DT))
    {
      world_generator.update(world, light_manager);
      player_controller.update(window, world, light_manager, FIXED_DT);
      light_manager.update(world);
      update_physics(world, FIXED_DT);
    }

    // 2: Rendering
    Camera camera;

    const Entity& player_entity = world.dimension.entities.at(world.player.entity_id);
    camera.transform            =  player_entity.transform;
    camera.transform.position.z += player_entity.eye;
    if(third_person)
      camera.transform.position -= player_entity.transform.local_forward() * 5.0f;

    int width, height;
    window.get_framebuffer_size(width, height);
    camera.aspect = static_cast<float>(width) / static_cast<float>(height);
    camera.fovy   = 45.0f;

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, width, height);

    world_renderer.render(camera, world, third_person, wireframer_renderer);
    player_controller.render(camera, world, wireframer_renderer);
    debug_renderer.render(window, world);

    window.swap_buffers();
  }
}
