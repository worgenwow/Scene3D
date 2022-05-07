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

    GLuint mVAO, mVBO, mIBO;
    void setupMesh();
  public:
    Mesh(std::vector<Vertex> &vertices, std::vector<GLuint> &indices);
    Mesh(std::vector<Vertex> &vertices, std::vector<GLuint> &indices, std::vector<Texture> &textures, Material material);

    void draw(Shader *shader);
    void addTexture(Texture texture);
};