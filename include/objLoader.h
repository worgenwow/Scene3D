#pragma once
#include <openglMaths.h>
#include <vector>
#include <string>
#include <model.h>
#include <obj_loader_structs.h>

class ObjLoader {
  private:
    bool loadObj(std::ifstream *fileStream, const std::string &directory, Model &model);
    
    bool checkLineEmpty(std::string &line, int currentLine, std::string fileType);

    std::vector<TextureMTL>  openMTL(std::string &string, const std::string &directory);
    std::vector<TextureMTL>  readMTL(std::ifstream *fileStream, const std::string &directory);

    void addMesh(std::vector<oglm::vec3> &positions, std::vector<oglm::vec3> &normals,
                 std::vector<oglm::vec2> &textureCoords, std::vector<Face> &faces, Model &model,
                 Material &material, std::vector<Texture> &textures);
    
    int getOneIndex(std::string &string, int &endIndex);
    VertexIndices getVertexIndices(std::string &string);
    Face getFace(std::string &string);

    float get1f(std::string &string);
    oglm::vec2 get2f(std::string &string);
    oglm::vec3 get3f(std::string &string);
  public:
    ObjLoader();

    bool loadObj(const std::string objPath, Model &model);
};