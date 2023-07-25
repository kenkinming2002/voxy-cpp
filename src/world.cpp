#include <world.hpp>

#include <perlin.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>

#include <math.h>

World::World(size_t seed) :
  camera{
    .position = glm::vec3(-5.0f, -5.0f,  50.0f),
    .aspect = 1024.0f / 720.0f,
    .yaw    = 45.0f,
    .pitch  = -45.0f,
    .fov    = 45.0f,
  },
  lights(Light(
    glm::vec3(0.0f, 0.0f, 30.0f), // position
    glm::vec3(0.2f, 0.2f, 0.2f),  // ambient
    glm::vec3(0.5f, 0.5f, 0.5f)   // diffuse
  )),
  terrain(seed)
{}


void World::handle_event(SDL_Event event)
{
  switch(event.type) {
    case SDL_MOUSEMOTION:
      camera.rotate(-event.motion.xrel, -event.motion.yrel);
      break;
    case SDL_MOUSEWHEEL:
      camera.zoom(-event.wheel.y);
      break;
  }
}

void World::update(float dt)
{
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
    translation = glm::normalize(translation);
    translation *= dt;
    camera.translate(translation.x, translation.y, translation.z);
  }

  terrain.load(camera.position, 500.0f);

  lights.update(dt);
  terrain.update(dt);
}

void World::render()
{
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  lights.render(camera);
  terrain.render(camera, lights);
}
