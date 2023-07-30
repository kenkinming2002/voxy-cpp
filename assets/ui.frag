#version 430 core
out vec4 outColor;

in vec2 fragTexCoords;

uniform sampler2D ourTexture;

void main()
{
  outColor = vec4(1.0, 1.0, 1.0, texture(ourTexture, fragTexCoords).r);
}
