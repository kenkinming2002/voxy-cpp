#version 430 core
layout (location = 0) in vec3 vertPos;
layout (location = 1) in vec2 vertNormal;
layout (location = 2) in vec2 vertTexCoords;

out vec2 fragNormal;
out vec2 fragTexCoords;

out float visibility;

uniform mat4 MVP;
uniform mat4 MV;

const float fogDensity  = 0.007;
const float fogGradient = 1.2;

void main()
{
  gl_Position = MVP * vec4(vertPos, 1.0);
  fragNormal    = vertNormal;
  fragTexCoords = vertTexCoords;

  // Fog
  vec4 position = MV * vec4(vertPos, 1.0);
  float dist = length(position.xyz);

  visibility = clamp(exp(-pow(dist * fogDensity, fogGradient)), 0.0, 1.0);
}
