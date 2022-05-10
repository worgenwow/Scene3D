#include <shader.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

void Shader::printShaderLog(GLuint shader) const {
	if(glIsShader(shader)) {
		// shader log length
		int infoLogLength = 0;
		int maxLength = infoLogLength;
		
		// get info string length
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
		
		// allocate string
		char *infoLog = new char[maxLength];
		
		// get info log
		glGetShaderInfoLog(shader, maxLength, &infoLogLength, infoLog);
		if(infoLogLength > 0) {
			std::cout << infoLog << std::endl;
		}

		delete[] infoLog;
	}
	else {
		std::cout << shader << " is not a shader." << std::endl;
	}
}

void Shader::printProgramLog(GLuint program) const {
	if(glIsProgram(program)) {
		// program log length
		int infoLogLength = 0;
		int maxLength = infoLogLength;
		
		// get info string length
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
		
		// allocate string
		char* infoLog = new char[maxLength];
		
		// get info log
		glGetProgramInfoLog(program, maxLength, &infoLogLength, infoLog);
		if(infoLogLength > 0)
		{
			std::cout << infoLog << std::endl;
		}
		
		delete[] infoLog;
	}	else {
		std::cout << program << " is not a program." << std::endl;
	}
}

Shader::Shader(const char *vertexPath, const char *fragmentPath, const char *geometryPath) {
  createShader(vertexPath, fragmentPath, geometryPath, true);
}

Shader::Shader(const char *vertexPath, const char *fragmentPath) {
  createShader(vertexPath, fragmentPath, "", false);
}

void Shader::createShader(const char *vertexPath, const char *fragmentPath, const char *geometryPath, bool useGeoShader) {
  std::string vertexCode;
  std::string fragmentCode;
  std::string geometryCode;
  std::ifstream vShaderFile;
  std::ifstream fShaderFile;
  std::ifstream gShaderFile;

  vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  try {
    // open the files
    vShaderFile.open(vertexPath);
    fShaderFile.open(fragmentPath);
    if(useGeoShader)
      gShaderFile.open(geometryPath);
    std::stringstream vShaderStream, fShaderStream, gShaderStream;

    // read the files buffer into the string streams
    vShaderStream << vShaderFile.rdbuf();
    fShaderStream << fShaderFile.rdbuf();
    if(useGeoShader)
      gShaderStream << gShaderFile.rdbuf();

    // close the file handlers
    vShaderFile.close();
    fShaderFile.close();
    if(useGeoShader)
      gShaderFile.close();

    // convert the stream into a string
    vertexCode   = vShaderStream.str();
    fragmentCode = fShaderStream.str();
    if(useGeoShader)
      geometryCode = gShaderStream.str();
  } catch(std::ifstream::failure e) {
    std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << e.what() << std::endl;
  }

  const GLchar *vShaderCode = vertexCode.c_str();
  const GLchar *fShaderCode = fragmentCode.c_str();
  const GLchar *gShaderCode;
  if(useGeoShader)
    gShaderCode = geometryCode.c_str();
  GLuint vShader, fShader, gShader;

  if(useGeoShader) {
    // compiling geometry shader
    gShader = glCreateShader(GL_GEOMETRY_SHADER);
    glShaderSource(gShader, 1, &gShaderCode, NULL);
    glCompileShader(gShader);

    GLint gShaderCompiled = GL_FALSE;
    glGetShaderiv(gShader, GL_COMPILE_STATUS, &gShaderCompiled);
    if(gShaderCompiled != GL_TRUE) {
      std::cout << "ERROR::SHADER::COMPILATION_FAILED of GEOMETRY" << std::endl;
      printShaderLog(gShader);
    }
  }

  // compiling vertex shader
  vShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vShader, 1, &vShaderCode, NULL);
  glCompileShader(vShader);

  GLint vShaderCompiled = GL_FALSE;
  glGetShaderiv(vShader, GL_COMPILE_STATUS, &vShaderCompiled);
  if(vShaderCompiled != GL_TRUE) {
    std::cout << "ERROR::SHADER::COMPILATION_FAILED of VERTEX" << std::endl;
    printShaderLog(vShader);
  }

  // compiling fragment shader
  fShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fShader, 1, &fShaderCode, NULL);
  glCompileShader(fShader);

  GLint fShaderCompiled = GL_FALSE;
  glGetShaderiv(fShader, GL_COMPILE_STATUS, &fShaderCompiled);
  if(fShaderCompiled != GL_TRUE) {
    std::cout << "ERROR::SHADER::COMPILATION_FAILED of FRAGMENT" << std::endl;
    printShaderLog(fShader);
  }

  // linking shader program
  mID = glCreateProgram();
  glAttachShader(mID, vShader);
  glAttachShader(mID, fShader);
  if(useGeoShader)
    glAttachShader(mID, gShader);
  glLinkProgram(mID);

  GLint programLinked = GL_FALSE;
  glGetProgramiv(mID, GL_LINK_STATUS, &programLinked);
  if(programLinked != GL_TRUE) {
    std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED" << std::endl;
    printProgramLog(mID);
  }

  glDeleteShader(vShader);
  glDeleteShader(fShader);
  if(useGeoShader)
    glDeleteShader(gShader);
}

void Shader::use() {
  glUseProgram(mID);
}

void Shader::bindUniformBlock(const char *name, const unsigned int binding) {
  GLuint index = glGetUniformBlockIndex(mID, name);
  glUniformBlockBinding(mID, index, binding);
}

// shader uniform functions
void Shader::setBool(const GLchar *name, bool value) const {
  glUniform1i(glGetUniformLocation(mID, name), (int)value);
}

void Shader::setInt(const GLchar *name, int value) const {
  glUniform1i(glGetUniformLocation(mID, name), value);
}

void Shader::setFloat(const GLchar *name, float value) const {
  glUniform1f(glGetUniformLocation(mID, name), value);
}

void Shader::setMat3(const GLchar *name, glm::mat3 &value) const {
  glUniformMatrix3fv(glGetUniformLocation(mID, name), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::setMat4(const GLchar *name, glm::mat4 &value) const {
  glUniformMatrix4fv(glGetUniformLocation(mID, name), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::setVec3(const GLchar *name, glm::vec3 &value) const {
  glUniform3fv(glGetUniformLocation(mID, name), 1, &value[0]);
}

void Shader::setVec3(const GLchar *name, float x, float y, float z) const {
  glUniform3f(glGetUniformLocation(mID, name), x, y, z);
}

void Shader::setVec4(const GLchar *name, glm::vec4 &value) const {
  glUniform4fv(glGetUniformLocation(mID, name), 1, &value[0]);
}

void Shader::setLightDropOff(const GLchar *name, LightDropOff &value) const {
  std::string baseName = name;

  setFloat((baseName + ".constant").c_str(), value.constant);
  setFloat((baseName + ".linear").c_str(), value.linear);
  setFloat((baseName + ".quadratic").c_str(), value.quadratic);
}

void Shader::setLightProps(const GLchar *name, LightProps &value) const {
  std::string baseName = name;

  setVec3((baseName + ".ambient").c_str(), value.ambient);
  setVec3((baseName + ".diffuse").c_str(), value.diffuse);
  setVec3((baseName + ".specular").c_str(), value.specular);
}

void Shader::setSpotlight(const GLchar *name, Spotlight &value) const {
  std::string baseName = name;

  setVec3((baseName + ".position").c_str(), value.position);
  setVec3((baseName + ".direction").c_str(), value.direction);
  setFloat((baseName + ".cutOff").c_str(), value.cutOff);
  setFloat((baseName + ".outerCutOff").c_str(), value.outerCutOff);
  setLightDropOff(name, value.lightDropOff);
  setLightProps(name, value.lightProps);
}

void Shader::setPointLight(const GLchar *name, PointLight &value) const {
  std::string baseName = name;

  setVec3((baseName + ".position").c_str(), value.position);
  setLightDropOff(name, value.lightDropOff);
  setLightProps(name, value.lightProps);
}

void Shader::setDirLight(const GLchar *name, DirLight &value) const {
  std::string baseName = name;

  setVec3((baseName + ".direction").c_str(), value.direction);
  setLightProps(name, value.lightProps);
}