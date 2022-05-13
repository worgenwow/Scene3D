#include <mesh.h>
#include <string>

Mesh::Mesh(std::vector<Vertex> &vertices, std::vector<GLuint> &indices) {
  mVertices = vertices;
  mIndices  = indices;

  setupMesh();
}

Mesh::Mesh(std::vector<Vertex> &vertices, std::vector<GLuint> &indices, std::vector<Texture> &textures, Material material) {
  mVertices = vertices;
  mIndices  = indices;
  mTextures = textures;
  mMaterial = material;

  setupMesh();
}

void Mesh::setupMesh() {
  glGenVertexArrays(1, &mVAO);
  glGenBuffers(1, &mVBO);
  glGenBuffers(1, &mIBO);

  glBindVertexArray(mVAO);

  glBindBuffer(GL_ARRAY_BUFFER, mVBO);
  glBufferData(GL_ARRAY_BUFFER, mVertices.size() * sizeof(Vertex), &mVertices[0], GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, mIndices.size() * sizeof(GLuint), &mIndices[0], GL_STATIC_DRAW);

  // vertex positions
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
  glEnableVertexAttribArray(0);
  // vertex normals
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
  glEnableVertexAttribArray(1);
  // vertex texture coords
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, textureCoords));
  glEnableVertexAttribArray(2);

  // unbind VAO and VBO after we're done  
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  // then unbind IBO
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Mesh::enableInstancing(oglm::vec3 *array, unsigned int arraySize) {
  glGenBuffers(1, &mInstanceVBO);

  glBindVertexArray(mVAO);

  glBindBuffer(GL_ARRAY_BUFFER, mInstanceVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(oglm::vec3) * arraySize, array, GL_STATIC_DRAW);

  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glVertexAttribDivisor(3, 1);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void Mesh::enableTextures(Shader *shader) {
  // textures have type texture_typeN, diffuseNr and specularNr are N for respective types
  unsigned int diffuseNr = 1;
  unsigned int specularNr = 1;
  for(unsigned int i=0; i < mTextures.size(); i++) {
    glActiveTexture(GL_TEXTURE0 + i); // activate texture unit

    std::string number;
    std::string type = mTextures[i].type;
    if(type == "texture_diffuse") {
      number = std::to_string(diffuseNr++); // var++ returns int then increments it
    } else if(type == "texture_specular") {
      number = std::to_string(specularNr++);
    }

    shader->setInt(("material."+type+number).c_str(), i);
    glBindTexture(GL_TEXTURE_2D, mTextures[i].ID);
  }
  // rebind default texture unit
  glActiveTexture(GL_TEXTURE0);
  shader->setFloat("material.specularExponent", mMaterial.specularExponent);
}

void Mesh::draw(Shader *shader) {
  enableTextures(shader);

  // draw mesh
  glBindVertexArray(mVAO);
  glDrawElements(GL_TRIANGLES, mIndices.size(), GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}

void Mesh::drawInstanced(Shader *shader, unsigned int amount) {
  enableTextures(shader);

  glBindVertexArray(mVAO);
  glDrawElementsInstanced(GL_TRIANGLES, mIndices.size(), GL_UNSIGNED_INT, 0, amount);
  glBindVertexArray(0);
}

void Mesh::addTexture(Texture texture) {
  mTextures.push_back(texture);
}