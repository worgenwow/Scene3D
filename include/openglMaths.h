#pragma once
#include <math.h>
#include <GL/glew.h>

namespace oglm {
  struct vec2;
  struct vec3;
  struct vec4;
  struct mat3;
  struct mat4;

  struct vec2{
    float x, y;

    vec2() { };
    vec2(float x, float y);
  };

  struct vec3{
    float x, y, z;

    vec3() { };
    vec3(float value);
    vec3(float x, float y, float z);
    vec3(vec4 vector);

    void getArray(GLfloat* vector) const;
    
    vec3 operator * (float f) const;
    vec3 operator * (vec3 vector) const;
    vec3 operator + (vec3 vector) const;
    vec3 operator - () const;
    vec3 operator - (vec3 vector) const;
    vec3& operator += (vec3 vector);
    vec3& operator -= (vec3 vector);
  };

  struct vec4{
    float x, y, z, w;

    vec4() { };
    vec4(float x, float y, float z, float w);
    vec4(vec3 vector);

    void getArray(GLfloat* vector) const;
  };

  // matrices are in column major order
  struct mat3{
    vec3 columns[3];

    mat3() { };
    mat3(vec3 vec0, vec3 vec1, vec3 vec2);
    mat3(float value);
    mat3(mat4 matrix);

    GLfloat* getArray() const;

    mat3 operator * (float f) const;
  };

  struct mat4{
    vec4 columns[4];

    mat4() { };
    mat4(vec4 vec0, vec4 vec1, vec4 vec2, vec4 vec3);
    mat4(float value);
    mat4(mat3 matrix);

    GLfloat* getArray() const;

    mat4 operator * (mat4 matrix) const;
  };

  mat3 transpose(mat3 matrix);
  mat3 inverse(mat3 matrix);
  mat4 translate(mat4 matrix, vec3 translation);
  mat4 rotate(mat4 matrix, float radians, vec3 axis);
  mat4 scale(mat4 matrix, vec3 scalar);
  mat4 lookAt(vec3 position, vec3 target, vec3 upVector);
  mat4 perspective(float fovy, float ratio, float nearClip, float farClip);
  float dot(vec3 left, vec3 right);
  vec3 normalize(vec3 vector);
  vec3 cross(vec3 left, vec3 right);
  float radians(float degrees);
};