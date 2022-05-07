#pragma once
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <string>

struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec2 textureCoords;
};

struct Texture {
  GLuint ID;
  const GLchar *type;
  std::string path;
};

struct Material {
  float specularExponent;
};