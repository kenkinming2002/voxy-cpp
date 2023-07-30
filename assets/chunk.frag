#version 430 core
out vec4 outColor;

in vec2       fragTexCoords;
flat in uint  fragTexIndex;
flat in float fragLightLevel;

uniform sampler2DArray blocksTextureArray;

void main()
{
  vec3 fragColor = texture(blocksTextureArray, vec3(fragTexCoords, float(fragTexIndex))).rgb;
  outColor = vec4(fragColor * fragLightLevel, 1.0);
}
