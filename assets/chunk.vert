#version 430 core
layout (location = 0) in vec3  aPos;
layout (location = 1) in vec3  aNormal;
layout (location = 2) in vec2  aUV;
layout (location = 3) in uint  aTextureIndex;
layout (location = 4) in float aLightLevel;

out vec3       fragPos;
out vec3       fragNormal;
out vec2       fragUV;
flat out uint  fragTextureIndex;
flat out float fragLightLevel;

uniform mat4 model;
uniform mat4 normal;
uniform mat4 MVP;

void main()
{
  gl_Position = MVP * vec4(aPos, 1.0);
  fragPos    = vec3(model  * vec4(aPos,    1.0));
  fragNormal = vec3(normal * vec4(aNormal, 0.0));

  fragUV           = aUV;
  fragTextureIndex = aTextureIndex;
  fragLightLevel   = aLightLevel;
}
