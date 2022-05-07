#pragma once
#include <glm/glm.hpp>

struct LightProps {
  glm::vec3 ambient;
  glm::vec3 diffuse;
  glm::vec3 specular;
};

// variables that handle light attenuation
struct LightDropOff {
  float constant;
  float linear;
  float quadratic;
};

struct Spotlight {
  glm::vec3 position;
  glm::vec3 direction;

  // cos of angle where outside light starts to dim
  float cutOff;
  // cos of angle where outside light is ambient
  float outerCutOff;

  LightDropOff lightDropOff;  
  LightProps lightProps;
};

struct DirLight {
  glm::vec3 direction;

  LightProps lightProps;
};

struct PointLight {
  glm::vec3 position;

  LightDropOff lightDropOff;
  LightProps lightProps;
};