#version 430 core
layout (location = 0) in vec2 inPosition;
layout (location = 1) in vec2 inTexCoords;

out vec2 fragTexCoords;

uniform mat4 MVP;

void main()
{
  gl_Position   = MVP * vec4(inPosition, 0.0, 1.0);
  fragTexCoords = inTexCoords;
}
