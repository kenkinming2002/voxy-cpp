#version 430 core
layout (location = 0) in vec3  vertPos;
layout (location = 1) in vec2  vertTexCoords;
layout (location = 2) in uint  vertTexIndex;
layout (location = 3) in float vertLightLevel;
layout (location = 4) in float vertDestroyLevel;

out vec2       fragTexCoords;
flat out uint  fragTexIndex;
flat out float fragLightLevel;
flat out float fragDestroyLevel;

uniform mat4 MVP;

void main()
{
  gl_Position = MVP * vec4(vertPos, 1.0);
  fragTexCoords    = vertTexCoords;
  fragTexIndex     = vertTexIndex;
  fragLightLevel   = vertLightLevel;
  fragDestroyLevel = vertDestroyLevel;
}
