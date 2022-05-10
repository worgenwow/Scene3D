#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform samplerCube skybox;
uniform vec3 viewPos;

void main() {
  float ratio = 1.00/1.52;
  vec3 I = normalize(FragPos - viewPos);
  vec3 R = refract(I, normalize(Normal), ratio);
  // vec3 R = reflect(I, normalize(Normal));
  FragColor = vec4(texture(skybox, R).rgb, 1.0);
}