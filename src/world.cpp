#include <world.hpp>

/*********
 * World *
 *********/
World::World() :
  m_camera{
    .aspect = 1024.0f / 720.0f,
    .fovy   = 45.0f,
  },
  m_player{
    .transform = {
      .position = glm::vec3(0.0f, 0.0f, 50.0f),
      .rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
    },
    .velocity     = glm::vec3(0.0f, 0.0f, 0.0f),
    .bounding_box = glm::vec3(0.9f, 0.9f, 1.9f),
  },
  m_dimension {
    .blocks_texture_array = TextureArray({
        "assets/stone.png",
        "assets/grass_bottom.png",
        "assets/grass_side.png",
        "assets/grass_top.png",
    }),
    .block_datas = {
      { .texture_indices = {0, 0, 0, 0, 0, 0} },
      { .texture_indices = {2, 2, 2, 2, 1, 3} },
    },
  }
{}

