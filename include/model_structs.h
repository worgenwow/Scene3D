#pragma once
#include <openglMaths.h>
#include <GL/glew.h>
#include <string>

struct Vertex {
  oglm::vec3 position;
  oglm::vec3 normal;
  oglm::vec2 textureCoords;
};

struct Texture {
  GLuint ID;
  const GLchar *type;
  std::string path;
};

struct Material {
  float specularExponent;
};