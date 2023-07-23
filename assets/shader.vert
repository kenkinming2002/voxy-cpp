#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;

out vec3 fragPos;
out vec3 fragNormal;
out vec3 fragColor;

layout (location = 0) uniform mat4 model;
layout (location = 1) uniform mat4 view;
layout (location = 2) uniform mat4 projection;

void main()
{
  gl_Position = projection * view * model * vec4(aPos, 1.0);
  fragPos    = vec3(model * vec4(aPos, 1.0));
  fragNormal = aNormal;
  fragColor  = aColor;
}
