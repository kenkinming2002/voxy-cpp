#version 430 core
out vec4 outColor;

in vec2 fragNormal;
in vec2 fragTexCoords;

in float visibility;

uniform sampler2D ourTexture;

const vec3 skyColor = vec3(0.2, 0.3, 0.3);

void main()
{
  vec3 fragColor = texture(ourTexture, fragTexCoords).rgb;
  outColor = vec4(mix(skyColor, fragColor, visibility), 1.0);
  outColor = vec4(fragColor, 1.0);
}

