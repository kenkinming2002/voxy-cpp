#version 430 core
out vec4 outColor;

in vec3 fragPos;
in vec3 fragNormal;
in vec3 fragColor;

uniform vec3 lightColor;
uniform vec3 lightPos;

uniform vec3 viewPos;

void main()
{
  float ambientStrength  = 0.1;
  float specularStrength = 0.5;

  vec3 normal = normalize(fragNormal);

  float ambient = ambientStrength;

  vec3 lightDir = normalize(fragPos - lightPos);
  float diffuse = max(dot(normal, lightDir), 0.0);

  vec3 viewDir    = normalize(fragPos - viewPos);
  vec3 reflectDir = reflect(lightDir, normal);
  float specular  = specularStrength * pow(max(dot(viewDir, reflectDir), 0.0), 32);

  vec3 result = (ambient + diffuse + specular) * lightColor * fragColor;
  outColor = vec4(result, 1.0);
}
