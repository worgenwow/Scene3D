#include <model.h>
#include <stdio.h>

Model::Model() {}

void Model::addMesh(Mesh mesh) {
  meshes.push_back(mesh);
}

void Model::draw(Shader *shader) {
  for(std::vector<Mesh>::iterator it = meshes.begin();
      it != meshes.end(); ++it) {
    it->draw(shader);
  }
}

std::vector<Texture> Model::loadTextures(TextureMTL &textureMTL) {
  std::vector<Texture> textures;

  bool diffuseLoaded = textureMTL.diffusePath.empty();
  bool specularLoaded = textureMTL.specularPath.empty();
  for(std::vector<Texture>::iterator it = loadedTextures.begin();
      it < loadedTextures.end(); ++it) {
    if(it->path == textureMTL.diffusePath) {
      textures.push_back(*it);
      diffuseLoaded = true;
    }
    if(it->path == textureMTL.specularPath) {
      textures.push_back(*it);
      specularLoaded = true;
    }
  }

  if(!diffuseLoaded) {
    Texture diffuse = textureFromFile(textureMTL.diffusePath);
    diffuse.type = "texture_diffuse";
    textures.push_back(diffuse);
    loadedTextures.push_back(diffuse);
  }
  if(!specularLoaded) {
    Texture specular = textureFromFile(textureMTL.specularPath);
    specular.type = "texture_specular";
    textures.push_back(specular);
    loadedTextures.push_back(specular);
  }

  return textures;
}

Texture Model::textureFromFile(const std::string &path) {
  Texture texture;
  texture.path = path;

  int width, height, nrChannels;
  unsigned char *data = ImageLoader::loadImage(path.c_str(), &width, &height, &nrChannels);

  GLenum format;
  if(nrChannels == 3) {
    format = GL_RGB;
  } else if(nrChannels == 4) {
    format = GL_RGBA;
  }

  if(data) {
    glGenTextures(1, &texture.ID);
    glBindTexture(GL_TEXTURE_2D, texture.ID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
  } else {
    printf("Failed to load texture");
  }
  ImageLoader::freeImage(data);

  return texture;
}