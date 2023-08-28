#pragma once

#include <graphics/camera.hpp>
#include <graphics/wireframe_renderer.hpp>

#include <world.hpp>

void render_player_ui(const graphics::Camera& camera, const World& world, graphics::WireframeRenderer& wireframe_renderer);
