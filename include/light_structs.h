#pragma once
#include <openglMaths.h>

struct LightProps {
  oglm::vec3 ambient;
  oglm::vec3 diffuse;
  oglm::vec3 specular;
};

// variables that handle light attenuation
struct LightDropOff {
  float constant;
  float linear;
  float quadratic;
};

struct Spotlight {
  oglm::vec3 position;
  oglm::vec3 direction;

  // cos of angle where outside light starts to dim
  float cutOff;
  // cos of angle where outside light is ambient
  float outerCutOff;

  LightDropOff lightDropOff;  
  LightProps lightProps;
};

struct DirLight {
  oglm::vec3 direction;

  LightProps lightProps;
};

struct PointLight {
  oglm::vec3 position;

  LightDropOff lightDropOff;
  LightProps lightProps;
};