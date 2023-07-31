#ifndef WORLD_HPP
#define WORLD_HPP

#include <camera.hpp>
#include <dimension.hpp>

/**********
 * Entity *
 **********/
struct Entity
{
  Transform transform;

  glm::vec3 velocity;
  glm::vec3 bounding_box;
};

/*********
 * World *
 *********/
struct World
{
public:
  World();

public:
  Dimension& dimension() { return m_dimension; }
  const Dimension& dimension() const { return m_dimension; }

  auto& camera() { return m_camera; }
  const auto& camera() const { return m_camera; }

  auto& player() { return m_player; }
  const auto& player() const { return m_player; }

private:
  Camera    m_camera;
  Entity    m_player;
  Dimension m_dimension;
};

#endif // WORLD_HPP
