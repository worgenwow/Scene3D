#pragma once
#include <mesh.h>
#include <vector>
#include <obj_loader_structs.h>
#include <imageLoader.h>

class Model {
  private:
    std::vector<Mesh> meshes;
    std::vector<Texture> loadedTextures;

    Texture textureFromFile(const std::string &path);
  public:
    Model();

    void addMesh(Mesh mesh);
    void draw(Shader *shader);
    std::vector<Texture> loadTextures(TextureMTL &textureMTL);
};