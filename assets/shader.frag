#version 430 core
out vec4 outColor;

in vec3 fragPos;
in vec3 fragNormal;
in vec3 fragColor;

layout (location = 3) uniform vec3 lightColor;
layout (location = 4) uniform vec3 lightPos;

void main()
{
  float ambientStrength = 0.1;

  vec3 normal           = normalize(fragNormal);
  vec3 lightDir         = normalize(fragPos - lightPos);
  float diffuseStrength = max(dot(normal, lightDir), 0.0);

  vec3 result = (ambientStrength + diffuseStrength) * lightColor * fragColor;
  outColor = vec4(result, 1.0);
}
