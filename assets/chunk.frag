#version 430 core
out vec4 outColor;

in vec3 fragPos;
in vec3 fragNormal;
in vec3 fragColor;

uniform vec3 viewPos;

uniform struct Light {
  vec3 pos;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
} light;

uniform struct Material {
  vec3  ambient;
  vec3  diffuse;
  vec3  specular;
  float shininess;
} material;

void main()
{
  float ambientStrength  = 0.1;
  float specularStrength = 0.5;

  vec3 normal = normalize(fragNormal);

  float ambient = ambientStrength;

  vec3 lightDir = normalize(fragPos - light.pos);
  float diffuse = max(-dot(normal, lightDir), 0.0);

  vec3 viewDir    = normalize(fragPos - viewPos);
  vec3 reflectDir = reflect(lightDir, normal);
  float specular  = specularStrength * pow(max(-dot(viewDir, reflectDir), 0.0), material.shininess);

  vec3 result = ambient  * light.ambient  * material.ambient
              + diffuse  * light.diffuse  * material.diffuse
              + specular * light.specular * material.specular;
  outColor = vec4(result, 1.0);
}
