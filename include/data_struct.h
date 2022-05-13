#pragma once
#include <GL/glew.h>
#include <math.h>

#include <shader.h>
#include <camera.h>
#include <key_data_struct.h>
#include <model.h>

struct Data {
  static const int shaderCount  = 4;
  Shader *shaders[shaderCount] = {};

  enum ShaderIndex{
    SCENE,
    VIEW_QUAD,
    SKYBOX,
    NORMALS_DEBUG
  };

  bool wireframe = false;

  GLuint instanceVBO;
  GLuint framebuffers[1];
  GLuint RBOs[1];
  GLuint textureColorBuffers[1];
  GLuint cubemap;
  GLuint skyboxVAO;
  GLuint matricesUBO;

  oglm::mat4 proj = oglm::mat4(1.0f);
  
  oglm::vec2 mouseChange = oglm::vec2(0,0);
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