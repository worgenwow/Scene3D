#pragma once
#include <GL/glew.h>
#include <openglMaths.h>
#include <light_structs.h>

class Shader {
  private:
    GLuint mID;

    // error printing
    void printShaderLog(GLuint shader) const;
    void printProgramLog(GLuint program) const;

    // creates program from shaders given
    void createShader(const char *vertexPath, const char *fragmentPath, const char *geometryPath, bool useGeoShader);
  public:
    Shader(const char *vertexPath, const char *fragmentPath, const char *geometryPath);
    Shader(const char *vertexPath, const char *fragmentPath);

    // sets program as active
    void use();

    void bindUniformBlock(const char *name, const unsigned int binding);
    // shader uniform functions
    void setBool(const GLchar *name, bool value) const;
    void setInt(const GLchar *name, int value) const;
    void setFloat(const GLchar *name, float value) const;
    void setMat3(const GLchar *name, oglm::mat3 &value) const;
    void setMat4(const GLchar *name, oglm::mat4 &value) const;
    void setVec3(const GLchar *name, oglm::vec3 &value) const;
    void setVec3(const GLchar *name, float x, float y, float z) const;
    void setVec4(const GLchar *name, oglm::vec4 &value) const;

    void setLightProps(const GLchar *name, LightProps &value) const;
    void setLightDropOff(const GLchar *name, LightDropOff &value) const;
    void setSpotlight(const GLchar *name, Spotlight &value) const;
    void setPointLight(const GLchar *name, PointLight &value) const;
    void setDirLight(const GLchar *name, DirLight &value) const;
};