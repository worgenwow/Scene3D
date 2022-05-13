#pragma once
#include <model_structs.h>
#include <vector>
#include <GL/glew.h>
#include <shader.h>

class Mesh {
  private:
    std::vector<Vertex> mVertices;
    std::vector<GLuint> mIndices;
    std::vector<Texture> mTextures;
    Material mMaterial;

    GLuint mVAO, mVBO, mIBO, mInstanceVBO;
    void setupMesh();
    void enableTextures(Shader *shader);
  public:
    Mesh(std::vector<Vertex> &vertices, std::vector<GLuint> &indices);
    Mesh(std::vector<Vertex> &vertices, std::vector<GLuint> &indices, std::vector<Texture> &textures, Material material);

    void enableInstancing(oglm::vec3 *array, unsigned int arraySize);

    void draw(Shader *shader);
    void drawInstanced(Shader *shader, unsigned int amount);
    void addTexture(Texture texture);
};