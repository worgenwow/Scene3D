#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <math.h>

#include <shader.h>
#include <camera.h>
#include <key_data_struct.h>
#include <model.h>

struct Data {
  static const int shaderCount  = 1;

  Shader *shaders[shaderCount] = {};

  enum shader_index{
    OBJECT
  };

  glm::mat4 proj = glm::mat4(1.0f);
  
  glm::vec2 mouseChange = glm::vec2(0,0);
  bool mouse1Down = false;

  float previousTime = 0.0f;
  int screenWidth  = 800;
  int screenHeight = 600;
  
  Camera camera;
  KeyData keyData;

  Model plane;
  Model cube;
  Model quad;
  Model backpack;

  ~Data() {
    for(int i=0;i < shaderCount;i++) {
      delete shaders[i];
    }
  }
};