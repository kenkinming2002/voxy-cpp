#pragma once

#include <graphics/shader_program.hpp>
#include <graphics/mesh.hpp>
#include <graphics/wireframe_renderer.hpp>

#include <application.hpp>
#include <world.hpp>
#include <camera.hpp>

#include <memory>

class PlayerController
{
public:
  static constexpr float ROTATION_SPEED = 0.1f;
  static constexpr float MOVEMENT_SPEED = 5.0f;

  static constexpr float JUMP_STRENGTH = 5.0f;

  static constexpr float RAY_CAST_LENGTH = 20.0f;
  static constexpr float ACTION_COOLDOWN = 0.1f;
  static constexpr float UI_SELECTION_THICKNESS = 3.0f;

public:
  PlayerController();

public:
  void update(Application& application, World& world, float dt);
  void render(const Camera& camera, const World& world, graphics::WireframeRenderer& wireframe_renderer);

private:
  bool   m_first = true;
  double m_cursor_xpos;
  double m_cursor_ypos;

  float m_cooldown = 0.0f;
};
