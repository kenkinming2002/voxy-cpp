#pragma once

#include <graphics/camera.hpp>
#include <graphics/mesh.hpp>
#include <graphics/shader_program.hpp>
#include <graphics/window.hpp>
#include <graphics/wireframe_renderer.hpp>

#include <world.hpp>

#include <light_manager.hpp>

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
  void update(World& world, LightManager& light_manager, float dt);
  void render(const graphics::Camera& camera, const World& world, graphics::WireframeRenderer& wireframe_renderer);

private:
  float m_cooldown = 0.0f;
};
