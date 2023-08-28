#include <world.hpp>

#include <world_generator.hpp>
#include <light_manager.hpp>
#include <physics.hpp>
#include <player_controller.hpp>

#include <graphics/camera.hpp>
#include <graphics/ui_renderer.hpp>
#include <graphics/window.hpp>
#include <graphics/wireframe_renderer.hpp>

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
  graphics::Camera            camera;
  graphics::WireframeRenderer wireframer_renderer;
  graphics::UIRenderer        ui_renderer;

  WorldRenderer world_renderer(load_resource_pack("resource_pack"));
  DebugRenderer debug_renderer;

  bool third_person = false;
  window.glfw_on_key([&third_person](int key, int scancode, int action, int mods) {
    if(key == GLFW_KEY_F5 && action == GLFW_PRESS)
      third_person = !third_person;
  });

  bool   cursor_first = false;
  double cursor_xpos;
  double cursor_ypos;

  Timer timer;
  for(;;)
  {
    window.poll_events();
    if(window.should_close())
      return 0;

    // 1: Update
    if(timer.tick(FIXED_DT))
    {
      // 1: Input Handling
      world.players.front().key_space = window.get_key(GLFW_KEY_SPACE) == GLFW_PRESS;
      world.players.front().key_w     = window.get_key(GLFW_KEY_W)     == GLFW_PRESS;
      world.players.front().key_a     = window.get_key(GLFW_KEY_A)     == GLFW_PRESS;
      world.players.front().key_s     = window.get_key(GLFW_KEY_S)     == GLFW_PRESS;
      world.players.front().key_d     = window.get_key(GLFW_KEY_D)     == GLFW_PRESS;

      world.players.front().mouse_button_left  = window.get_mouse_button(GLFW_MOUSE_BUTTON_LEFT)  == GLFW_PRESS;
      world.players.front().mouse_button_right = window.get_mouse_button(GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;

      double new_cursor_xpos;
      double new_cursor_ypos;
      window.get_cursor_pos(new_cursor_xpos, new_cursor_ypos);
      if(cursor_first)
      {
        world.players.front().cursor_motion_x = 0.0f;
        world.players.front().cursor_motion_y = 0.0f;
      }
      else
      {
        world.players.front().cursor_motion_x = new_cursor_xpos - cursor_xpos;
        world.players.front().cursor_motion_y = new_cursor_ypos - cursor_ypos;
      }
      cursor_xpos = new_cursor_xpos;
      cursor_ypos = new_cursor_ypos;

      // 2: Actual Update
      world_generator.update(world, light_manager);
      player_controller.update(world, light_manager, FIXED_DT);
      light_manager.update(world);
      update_physics(world, FIXED_DT);
    }

    // 2: Rendering
    const Player& player        = world.players.front();
    const Entity& player_entity = world.entities.at(player.entity_id);
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
    debug_renderer.render(glm::vec2(width, height), world, ui_renderer);

    window.swap_buffers();
  }
}
