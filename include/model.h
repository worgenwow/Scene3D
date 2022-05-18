#pragma once
#include <mesh.h>
#include <vector>
#include <obj_loader_structs.h>
#include <imageLoader.h>

class Model {
  private:
    std::vector<Mesh> meshes;
    std::vector<Texture> loadedTextures;

    Texture textureFromFile(const std::string &path, bool gammaCorrect);
  public:
    Model();

    void addMesh(Mesh mesh);

    void enableInstancing(oglm::vec3 *array, unsigned int arraySize);

    void draw(Shader *shader);
    void drawInstanced(Shader *shader, unsigned int amount);
    std::vector<Texture> loadTextures(TextureMTL &textureMTL);
};