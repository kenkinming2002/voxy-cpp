#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <timer.hpp>
#include <window.hpp>

#include <world.hpp>

#include <system/chunk_generator.hpp>
#include <system/chunk_mesher.hpp>
#include <system/chunk_renderer.hpp>

#include <system/light.hpp>
#include <system/physics.hpp>

#include <system/player_control.hpp>
#include <system/player_ui.hpp>

#include <system/camera_follow.hpp>

#include <system/debug.hpp>


class Application
{
public:
  Application();

public:
  void run();

private:
  void loop();

private:
  bool m_running;

private:
  Window m_window;
  Timer  m_timer;

private:
  World  m_world;

private:
  std::unique_ptr<ChunkGeneratorSystem> m_chunk_generator_system;
  std::unique_ptr<ChunkMesherSystem>    m_chunk_mesher_system;
  std::unique_ptr<ChunkRendererSystem>  m_chunk_renderer_system;

  std::unique_ptr<LightSystem>   m_light_system;
  std::unique_ptr<PhysicsSystem> m_physics_system;

  std::unique_ptr<PlayerControlSystem> m_player_control_system;
  std::unique_ptr<PlayerUISystem>      m_player_ui_system;
  std::unique_ptr<CameraFollowSystem>  m_camera_follow_system;

  std::unique_ptr<DebugSystem> m_debug_system;
};

#endif // APPLICATION_HPP
