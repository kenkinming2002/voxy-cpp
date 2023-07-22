#version 430 core
out vec4 outColor;

in vec2 fragUV;

uniform sampler2D texture0;
uniform sampler2D texture1;

void main()
{
  outColor = mix(texture(texture0, fragUV), texture(texture1, fragUV), 0.2);
}
