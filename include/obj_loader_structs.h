#pragma once
#include <string>
#include <vector>

struct VertexIndices {
  int positionIndex = -1;
  int normalIndex = -1;
  int textureCoordIndex = -1;
};

struct Face {
  std::vector<VertexIndices> vertexIndices;
};

struct TextureMTL {
  std::string name;
  float specularExponent;
  std::string diffusePath;
  std::string specularPath;

  TextureMTL() {
    name.clear();
    diffusePath.clear();
    specularPath.clear();
    specularExponent = 0.0f;
  }
};