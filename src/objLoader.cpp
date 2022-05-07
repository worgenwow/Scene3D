#include <objLoader.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

ObjLoader::ObjLoader(){}

// returns true on successful object load
bool ObjLoader::loadObj(const std::string objPath, Model &model) {
  std::ifstream file;
  bool success;

  // get directory to load other files
  std::string directory = objPath.substr(0, objPath.find_last_of("/")+1);

  file.exceptions(std::ifstream::badbit);
  try {
    file.open(objPath);

    if(file.is_open())
      success = loadObj(&file, directory, model);
    else
      success = false;

    file.close();
  } catch (std::ifstream::failure e) {
    std::cout << "ERROR::OBJ_LOADER::OBJ::FILE_NOT_SUCCESFULLY_READ: " << e.what() << std::endl;
    success = false;
  }
  return success;
}

// returns true on successful object load
bool ObjLoader::loadObj(std::ifstream *fileStream, const std::string &directory, Model &model) {
  std::vector<glm::vec3> positions;
  std::vector<glm::vec3> normals;
  std::vector<glm::vec2> textureCoords;
  std::vector<Face> faces;
  std::vector<TextureMTL> texturesMTL;
  std::vector<Texture> textures;
  Material material;

  int currentLine = 0;
  std::string line;
  while(!fileStream->eof()) {
    checkLineEmpty(line, currentLine++, "OBJ"); // var++ returns current value then increments it
    std::getline(*fileStream, line);

    // if empty line continue
    if(line.empty()) continue;

    // skip leading space
    if(line[0] == ' ' || line[0] == '\t') {
      line.erase(0);
    }

    if(line[0] == '\0') { // empty line
      line.clear();
      continue;
    }
    if(line[0] == '#') {  // comment line
      line.clear();
      continue;
    }

    // handle vertex line
    if(line[0] == 'v' && (line[1] == ' ' || line[1] == '\t')) {
      line.erase(0, 2);
      
      glm::vec3 vertex = get3f(line);
      positions.push_back(vertex);
      continue;
    }

    // handle vertex normal line
    if(line[0] == 'v' && line[1] == 'n' && (line[2] == ' ' || line[2] == '\t')) {
      line.erase(0, 3);

      glm::vec3 vertexNormal = get3f(line);
      normals.push_back(vertexNormal);
      continue;
    }

    // handle texture coordinate line
    if(line[0] == 'v' && line[1] == 't' && (line[2] == ' ' || line[2] == '\t')) {
      line.erase(0, 3);

      glm::vec2 textureCoord = get2f(line);
      textureCoords.push_back(textureCoord);
      continue;
    }

    // handle face line
    if(line[0] == 'f' && (line[1] == ' ' || line[1] == '\t')) {
      line.erase(0, 2);
      
      Face face = getFace(line);
      faces.push_back(face);
      continue;
    }

    // handle object line
    if(line[0] == 'o' && (line[1] == ' ' || line[1] == '\t')) {
      line.erase(0, 2);
      
      addMesh(positions, normals, textureCoords, faces, model, material, textures);
      //@TODO name objects
      line.clear();
      continue;
    }

    // handle usemtl line
    if((line.substr(0,6).compare("usemtl") == 0) && (line[6] == ' ' || line[6] == '\t')) {
      line.erase(0, 7);

      for(std::vector<TextureMTL>::iterator it = texturesMTL.begin();
          it < texturesMTL.end(); ++it) {
        if(it->name == line) {
          material.specularExponent = it->specularExponent;
          textures = model.loadTextures(*it);
          break;
        }
      }
      line.clear();
      continue;
    }

    // handle mtllib line
    if((line.substr(0,6).compare("mtllib") == 0) && (line[6] == ' ' || line[6] == '\t')) {
      line.erase(0, 7);

      texturesMTL = openMTL(line, directory); // get MTL texures from file
      line.clear();
      continue;
    }

    //handle s line????
    if(line[0] == 's' && (line[1] == ' ' || line[1] == '\t')) {
      line.erase(0, 2);
      //@TODO
      line.clear();
      continue;
    }

    std::cout << "WARNING::OBJ_LOADER::OBJ::LINE_UNHANDLED at: {"<< currentLine <<"}\nLINE: {" << line << "}" << std::endl;
    line.clear();
  }

  addMesh(positions, normals, textureCoords, faces, model, material, textures);
  return true;
}

// adds a mesh to the model being loaded
void ObjLoader::addMesh(std::vector<glm::vec3> &positions, std::vector<glm::vec3> &normals,
                        std::vector<glm::vec2> &textureCoords, std::vector<Face> &faces, Model &model,
                        Material &material, std::vector<Texture> &textures) {
  if(positions.empty() || normals.empty() || textureCoords.empty() || faces.empty()) return;
  
  std::vector<Vertex> vertices;
  std::vector<GLuint> indices;
  GLuint index = 0;
  for(std::vector<Face>::iterator it = faces.begin(); it < faces.end(); ++it) { // iterate over faces
    for(std::vector<VertexIndices>::iterator jt = it->vertexIndices.begin();
        jt < it->vertexIndices.end(); ++jt) { // iterate over vertices in face
      Vertex vertex;
      vertex.position = positions[jt->positionIndex];
      vertex.normal = normals[jt->normalIndex];
      vertex.textureCoords = textureCoords[jt->textureCoordIndex];

      vertices.push_back(vertex); // add the vertices to the vertex list
      indices.push_back(index++);
    }
  }
  if(textures.empty()) {
    Mesh mesh(vertices, indices);
    model.addMesh(mesh);
  } else {
    Mesh mesh(vertices, indices, textures, material);
    textures.clear();
    model.addMesh(mesh);
  }
  
  faces.clear();
}

// gets all the vertex data for a face
Face ObjLoader::getFace(std::string &string) {
  Face face;

  while(!string.empty()) {
    face.vertexIndices.push_back(getVertexIndices(string));
  }

  return face;
}

// gets the indices to the vertex data for a face vertex
VertexIndices ObjLoader::getVertexIndices(std::string &string) {
  VertexIndices vertexIndices;

  int endIndex = string.find(' ');
  if(endIndex == -1) {
    endIndex = string.size();
  }
  vertexIndices.positionIndex = getOneIndex(string, endIndex);
  vertexIndices.textureCoordIndex = getOneIndex(string, endIndex);
  vertexIndices.normalIndex = getOneIndex(string, endIndex);

  return vertexIndices;
}

// gets one index integer from string
int ObjLoader::getOneIndex(std::string &string, int &endIndex) {
  int result = -1;

  if(endIndex == -1) { // if already reached end return
    return result;
  }

  int slashIndex = string.find('/');
  if(slashIndex == std::string::npos) { // if end of string found read final int
    result = std::stoi(string);
    string.clear();
  } else if(slashIndex == 0) {          // if slash at 0 return -1 and remove
    string.erase(0, 1);
    endIndex -= 1; // we removed an element
    return result;
  } else {
    if(slashIndex > endIndex) {         // if slash is after end just read final int
      result = std::stoi(string.substr(0, endIndex));
      string.erase(0, endIndex+1);
      endIndex = -1;
    } else {                            // read int up to slash
      result = std::stoi(string.substr(0, slashIndex));  // if int is 3 long slash index = 3 as index starts at 0
      string.erase(0, slashIndex+1);                     // want to delete slash so remove 4
      endIndex -= slashIndex+1;                          // we removed 4 elements
    }
  }
  return result - 1;
}

// gets 3 floats from string
glm::vec3 ObjLoader::get3f(std::string &string) {
  glm::vec3 floats;

  floats.x = get1f(string);
  floats.y = get1f(string);
  floats.z = get1f(string);
  return floats;
}

// gets 2 floats from string
glm::vec2 ObjLoader::get2f(std::string &string) {
  glm::vec2 floats;

  floats.x = get1f(string);
  floats.y = get1f(string);
  return floats;
}

// gets next float from string then removes it
float ObjLoader::get1f(std::string &string) {
  int index = string.find(' ');
  float result;

  if(index == std::string::npos) { 
    result = std::stof(string);                  // if no space found last float is up to end of line
    string.clear();
  } else {
    result = std::stof(string.substr(0, index)); // otherwise gets data up to space to turn into float
    string.erase(0, index + 1);                  // remove float and space from line
  }
  return result;
}

// checks if line was fully emptied if not outputs debug and clears line
bool ObjLoader::checkLineEmpty(std::string &line, int currentLine, std::string fileType) {
  if(line.empty()) {
    return true;
  } else {
    std::cout << "WARNING::OBJ_LOADER::" << fileType << "::PARTIAL_LINE_HANDLE at: {"<< currentLine <<"}\nLINE: {" << line << "}" << std::endl;
    line.clear();
    return false;
  }
}

std::vector<TextureMTL>  ObjLoader::openMTL(std::string &string, const std::string &directory) {
  std::vector<TextureMTL> textures;
  std::ifstream mtlStream;
  mtlStream.exceptions(std::ifstream::badbit);
  try {
    mtlStream.open(directory + string);

    if(mtlStream.is_open())
      textures = readMTL(&mtlStream, directory);

    mtlStream.close();
  } catch (std::ifstream::failure e) {
    std::cout << "ERROR::OBJ_LOADER::MTL::FILE_NOT_SUCCESFULLY_READ: " << e.what() << std::endl;
  }

  return textures;
}

std::vector<TextureMTL> ObjLoader::readMTL(std::ifstream *fileStream, const std::string &directory) {
  std::vector<TextureMTL> textures;
  int currentLine = 0;
  std::string line;
  TextureMTL texture;
  while(!fileStream->eof()) {
    checkLineEmpty(line, currentLine++, "MTL");
    std::getline(*fileStream, line);

    // if empty line continue
    if(line.empty()) continue;

    // skip leading space
    if(line[0] == ' ' || line[0] == '\t') {
      line.erase(0);
    }

    if(line[0] == '\0') { // empty line
      line.clear();
      continue;
    }
    if(line[0] == '#') {  // comment line
      line.clear();
      continue;
    }

    // handle specular exponent line
    if(line[0] == 'N' && line[1] == 's' && (line[2] == ' ' || line[2] == '\t')) {
      line.erase(0, 3);
      
      texture.specularExponent = std::stof(line);
      line.clear();
      continue;
    }

    // handle ambient color line
    if(line[0] == 'K' && line[1] == 'a' && (line[2] == ' ' || line[2] == '\t')) {
      line.erase(0, 3);
      //@TODO
      line.clear();
      continue;
    }

    // handle diffuse color line
    if(line[0] == 'K' && line[1] == 'd' && (line[2] == ' ' || line[2] == '\t')) {
      line.erase(0, 3);
      //@TODO
      line.clear();
      continue;
    }

    // handle specular color line
    if(line[0] == 'K' && line[1] == 's' && (line[2] == ' ' || line[2] == '\t')) {
      line.erase(0, 3);
      //@TODO
      line.clear();
      continue;
    }

    // handle emmisive color line
    if(line[0] == 'K' && line[1] == 'e' && (line[2] == ' ' || line[2] == '\t')) {
      line.erase(0, 3);
      //@TODO
      line.clear();
      continue;
    }

    // handle index of refraction / optical density line
    if(line[0] == 'N' && line[1] == 'i' && (line[2] == ' ' || line[2] == '\t')) {
      line.erase(0, 3);
      //@TODO
      line.clear();
      continue;
    }

    // handle dissolve line
    if(line[0] == 'd' && (line[1] == ' ' || line[1] == '\t')) {
      line.erase(0, 3);
      //@TODO
      line.clear();
      continue;
    }

    // handle newmtl line
    if((line.substr(0,6).compare("newmtl") == 0) && (line[6] == ' ' || line[6] == '\t')) {
      line.erase(0, 7);
      
      if(!texture.name.empty()) {
         textures.push_back(texture);
         texture = TextureMTL();
      }

      texture.name = line;
      line.clear();
      continue;
    }

    // handle diffuse map line
    if((line.substr(0,6).compare("map_Kd") == 0) && (line[6] == ' ' || line[6] == '\t')) {
      line.erase(0, 7);
      
      texture.diffusePath = directory + line;
      line.clear();
      continue;
    }

    // handle specular map line
    if((line.substr(0,6).compare("map_Ks") == 0) && (line[6] == ' ' || line[6] == '\t')) {
      line.erase(0, 7);
      
      texture.specularPath = directory + line;
      line.clear();
      continue;
    }

    // handle bump map line
    if((line.substr(0,8).compare("map_Bump") == 0) && (line[8] == ' ' || line[8] == '\t')) {
      line.erase(0, 9);
      //@TODO
      line.clear();
      continue;
    }

    // handle illumination model line
    if((line.substr(0,5).compare("illum") == 0) && (line[5] == ' ' || line[5] == '\t')) {
      line.erase(0, 6);
      //@TODO
      line.clear();
      continue;
    }

    std::cout << "WARNING::OBJ_LOADER::OBJ::LINE_UNHANDLED at: {"<< currentLine <<"}\nLINE: {" << line << "}" << std::endl;
    line.clear();
  }

  textures.push_back(texture);
  return textures;
}