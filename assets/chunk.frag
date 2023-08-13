#version 430 core
out vec4 outColor;

in vec2       fragTexCoords;
flat in uint  fragTexIndex;
flat in float fragLightLevel;
flat in float fragDestroyLevel;

uniform sampler2DArray blocksTextureArray;

float rand(vec2 value1)
{
  return fract(sin(dot(value1, vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{
  vec2  value1 = floor(fragTexCoords * 16.0) * 16.0;
  float value2 = fragDestroyLevel;
  float darken_factor = rand(value1) * fragDestroyLevel;
  float darken = floor(darken_factor / 0.2) * 0.15;

  vec3 fragColor = texture(blocksTextureArray, vec3(fragTexCoords, float(fragTexIndex))).rgb;
  outColor = vec4(fragColor * fragLightLevel * (1.0 - darken), 1.0);
}
