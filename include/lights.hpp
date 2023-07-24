#ifndef LIGHTS_HPP
#define LIGHTS_HPP

#include <gl.hpp>
#include <light.hpp>
#include <camera.hpp>

struct Lights
{
  gl::Program program;
  Light       light;

  // TODO: Allow more than one light
  Lights(Light light);

  void update(float dt);
  void render(const Camera& camera);
};

#endif // LIGHTS_HPP
