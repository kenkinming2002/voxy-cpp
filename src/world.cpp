#include <world.hpp>

#include <chunk_coords.hpp>

#include <glm/gtx/norm.hpp>

#include <fmt/format.h>
#include <spdlog/spdlog.h>

/*************
 * Constants *
 *************/
static constexpr float ROTATION_SPEED = 0.1f;

static constexpr glm::vec2   DEBUG_MARGIN       = glm::vec2(3.0f, 3.0f);
static constexpr const char *DEBUG_FONT        = "assets/arial.ttf";
static constexpr float       DEBUG_FONT_HEIGHT = 20.0f;

/*********
 * World *
 *********/
World::World(std::size_t seed) :
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
  },
  m_text_renderer(DEBUG_FONT, DEBUG_FONT_HEIGHT),
  m_chunk_generator_system(ChunkGeneratorSystem::create(seed)),
  m_chunk_mesher_system(ChunkMesherSystem::create()),
  m_chunk_renderer_system(ChunkRendererSystem::create()),
  m_light_system(LightSystem::create()),
  m_physics_system(PhysicsSystem::create())
{}

void World::handle_event(SDL_Event event)
{
  switch(event.type)
  {
    case SDL_MOUSEMOTION:
      m_player.transform = m_player.transform.rotate(glm::vec3(0.0f,
        -event.motion.yrel * ROTATION_SPEED,
        -event.motion.xrel * ROTATION_SPEED
      ));
      break;
    case SDL_MOUSEWHEEL:
      m_camera.zoom(-event.wheel.y);
      break;
  }
}

void World::update(float dt)
{
  // 1: Camera Movement
  glm::vec3 translation = glm::vec3(0.0f);

  const Uint8 *keys = SDL_GetKeyboardState(nullptr);
  if(keys[SDL_SCANCODE_SPACE])  translation.z += 1.0f;
  if(keys[SDL_SCANCODE_LSHIFT]) translation.z -= 1.0f;
  if(keys[SDL_SCANCODE_W])      translation.y += 1.0f;
  if(keys[SDL_SCANCODE_S])      translation.y -= 1.0f;
  if(keys[SDL_SCANCODE_D])      translation.x += 1.0f;
  if(keys[SDL_SCANCODE_A])      translation.x -= 1.0f;
  if(glm::length(translation) != 0.0f)
  {
    translation = m_player.transform.gocal_to_global(translation);
    translation = glm::normalize(translation);
    translation *= dt;
    m_player.velocity += translation * 10.0f;
  }

  // 2: Update
  m_light_system->update(*this);
  m_chunk_generator_system->update(*this);
  m_chunk_mesher_system->update(*this);
  m_physics_system->update(*this, dt);

  // 3: Camera update
  m_camera.transform           = m_player.transform;
  m_camera.transform.position += glm::vec3(0.5f, 0.5f, 1.5f);
}

void World::render()
{
  m_chunk_renderer_system->render(*this);

  std::string line;
  glm::vec2   cursor = DEBUG_MARGIN;

  line = fmt::format("position: x = {}, y = {}, z = {}", m_player.transform.position.x, m_player.transform.position.y, m_player.transform.position.z);
  m_text_renderer.render(cursor, line.c_str());
  cursor.x = DEBUG_MARGIN.x;
  cursor.y += DEBUG_FONT_HEIGHT;

  line = fmt::format("velocity: x = {}, y = {}, z = {}", m_player.velocity.x, m_player.velocity.y, m_player.velocity.z);
  m_text_renderer.render(cursor, line.c_str());
  cursor.x = DEBUG_MARGIN.x;
  cursor.y += DEBUG_FONT_HEIGHT;
}

