#include <openglMaths.h>
#include <stdio.h>

oglm::mat3 oglm::transpose(mat3 matrix) {
  mat3 result;
  result.columns[0] = vec3(matrix.columns[0].x, matrix.columns[1].x, matrix.columns[2].x);
  result.columns[1] = vec3(matrix.columns[0].y, matrix.columns[1].y, matrix.columns[2].y);
  result.columns[2] = vec3(matrix.columns[0].z, matrix.columns[1].z, matrix.columns[2].z);

  return result;
}

oglm::mat3 oglm::inverse(mat3 matrix) {
  float det  = matrix.columns[0].x * ((matrix.columns[1].y * matrix.columns[2].z) - (matrix.columns[2].y * matrix.columns[1].z));
        det -= matrix.columns[1].x * ((matrix.columns[0].y * matrix.columns[2].z) - (matrix.columns[2].y * matrix.columns[0].z));
        det += matrix.columns[2].x * ((matrix.columns[0].y * matrix.columns[1].z) - (matrix.columns[1].y * matrix.columns[0].z));

  // if(det == 0) {
  //   printf("No inverse exists\n");
  //   return mat3();
  // }

  mat3 inverse;
  inverse.columns[0].x =  ((matrix.columns[1].y * matrix.columns[2].z) - (matrix.columns[2].y * matrix.columns[1].z));
  inverse.columns[1].x = -((matrix.columns[0].y * matrix.columns[2].z) - (matrix.columns[2].y * matrix.columns[0].z));
  inverse.columns[2].x =  ((matrix.columns[0].y * matrix.columns[1].z) - (matrix.columns[1].y * matrix.columns[0].z));

  inverse.columns[0].y = -((matrix.columns[1].x * matrix.columns[2].z) - (matrix.columns[2].x * matrix.columns[1].z));
  inverse.columns[1].y =  ((matrix.columns[0].x * matrix.columns[2].z) - (matrix.columns[2].x * matrix.columns[0].z));
  inverse.columns[2].y = -((matrix.columns[0].x * matrix.columns[1].z) - (matrix.columns[1].x * matrix.columns[0].z));

  inverse.columns[0].z =  ((matrix.columns[1].x * matrix.columns[2].y) - (matrix.columns[2].x * matrix.columns[1].y));
  inverse.columns[1].z = -((matrix.columns[0].x * matrix.columns[2].y) - (matrix.columns[2].x * matrix.columns[0].y));
  inverse.columns[2].z =  ((matrix.columns[0].x * matrix.columns[1].y) - (matrix.columns[1].x * matrix.columns[0].y));

  inverse = transpose(inverse);
  inverse = inverse * (1/det);
  return inverse;
}

oglm::mat4 oglm::translate(mat4 matrix, vec3 translation) {
  mat4 translationMat;
  translationMat.columns[0] = vec4(1.0f, 0.0f, 0.0f, 0.0f);
  translationMat.columns[1] = vec4(0.0f, 1.0f, 0.0f, 0.0f);
  translationMat.columns[2] = vec4(0.0f, 0.0f, 1.0f, 0.0f);
  translationMat.columns[3] = vec4(translation.x, translation.y, translation.z, 1.0f);

  return matrix * translationMat;
}

oglm::mat4 oglm::rotate(mat4 matrix, float radians, vec3 axis) {
  float x = axis.x;
  float y = axis.y;
  float z = axis.z;
  float cosr = cos(radians);
  float sinr = sin(radians);

  mat4 rotationMat;

  rotationMat.columns[0].x = x*x*(1-cosr) + 1*cosr;
  rotationMat.columns[1].x = x*y*(1-cosr) - z*sinr;
  rotationMat.columns[2].x = x*z*(1-cosr) + y*sinr;
  

  rotationMat.columns[0].y = y*x*(1-cosr) + z*sinr;
  rotationMat.columns[1].y = y*y*(1-cosr) + 1*cosr;
  rotationMat.columns[2].y = y*z*(1-cosr) - x*sinr;
  

  rotationMat.columns[0].z = z*x*(1-cosr) - y*sinr;
  rotationMat.columns[1].z = z*y*(1-cosr) + x*sinr;
  rotationMat.columns[2].z = z*z*(1-cosr) + 1*cosr;

  rotationMat.columns[0].w = 0.0f;
  rotationMat.columns[1].w = 0.0f;
  rotationMat.columns[2].w = 0.0f;

  rotationMat.columns[3] = vec4(0.0f, 0.0f, 0.0f, 1.0f);

  return matrix * rotationMat;
}

oglm::mat4 oglm::scale(mat4 matrix, vec3 scalar) {
  mat4 scalarMat;
  scalarMat.columns[0] = vec4(scalar.x, 0.0f, 0.0f, 0.0f);
  scalarMat.columns[1] = vec4(0.0f, scalar.y, 0.0f, 0.0f);
  scalarMat.columns[2] = vec4(0.0f, 0.0f, scalar.z, 0.0f);
  scalarMat.columns[3] = vec4(0.0f, 0.0f, 0.0f, 1.0f);

  return matrix * scalarMat;
}

oglm::mat4 oglm::lookAt(vec3 position, vec3 target, vec3 upVector) {
  vec3 forward = normalize(target - position);
  vec3 right = normalize(cross(forward, upVector));
  vec3 up = normalize(cross(right, forward));

  mat4 result;
  result.columns[0] = vec4(right.x, up.x, -forward.x, 0.0f);
  result.columns[1] = vec4(right.y, up.y, -forward.y, 0.0f);
  result.columns[2] = vec4(right.z, up.z, -forward.z, 0.0f);
  result.columns[3] = vec4(-dot(right, position), -dot(upVector, position), dot(forward, position), 1.0f);

  return result;
}

oglm::mat4 oglm::perspective(float fovy, float ratio, float nearClip, float farClip) {
  float tanHF = tan(fovy/2.f);
  mat4 perspective(0.0f);
  perspective.columns[0].x = 1.f/(ratio * tanHF);
  perspective.columns[1].y = 1.f/tanHF;
  perspective.columns[2].z = -(farClip + nearClip)/(farClip - nearClip);
  perspective.columns[2].w = -1.f;
  perspective.columns[3].z = -(2 * farClip * nearClip)/(farClip - nearClip);
  return perspective;
}

float oglm::dot(vec3 left, vec3 right) {
  return (left.x * right.x) + (left.y * right.y) + (left.z * right.z);
}

oglm::vec3 oglm::normalize(vec3 vector) {
  float magnitude = sqrt(pow(vector.x,2) + pow(vector.y,2) + pow(vector.z,2));
  return vec3(vector.x/magnitude, vector.y/magnitude, vector.z/magnitude);
}

oglm::vec3 oglm::cross(vec3 left, vec3 right) {
  vec3 result;
  result.x = (left.y * right.z) - (left.z * right.y);
  result.y = (left.z * right.x) - (left.x * right.z);
  result.z = (left.x * right.y) - (left.y * right.x);
  return result;
}

float oglm::radians(float degrees) {
  return M_PI * (degrees/180.f);
}

oglm::vec2::vec2(float x, float y) {
  this->x = x;
  this->y = y;
}

oglm::vec3::vec3(float x, float y, float z) {
  this->x = x;
  this->y = y;
  this->z = z;
}

oglm::vec3::vec3(float value) {
  this->x = value;
  this->y = value;
  this->z = value;
}

oglm::vec3::vec3(vec4 vector) {
  x = vector.x;
  y = vector.y;
  z = vector.z;
}

oglm::vec3 oglm::vec3::operator * (float f) const {
  vec3 result = vec3(x * f, y * f, z * f);
  return result;
}

oglm::vec3 oglm::vec3::operator * (vec3 vector) const {
  vec3 result = vec3(x * vector.x, y * vector.y, z * vector.z);
  return result;
}

oglm::vec3 oglm::vec3::operator + (vec3 vector) const {
  vec3 result = vec3(x + vector.x, y + vector.y, z + vector.z);
  return result;
}

oglm::vec3 oglm::vec3::operator - () const {
  vec3 result = vec3(-x, -y, -z);
  return result;
}

oglm::vec3 oglm::vec3::operator - (vec3 vector) const {
  vec3 result = vec3(x - vector.x, y - vector.y, z - vector.z);
  return result;
}

oglm::vec3& oglm::vec3::operator += (vec3 vector) {
  x += vector.x;
  y += vector.y;
  z += vector.z;
  return *this;
}

oglm::vec3& oglm::vec3::operator -= (vec3 vector) {
  x -= vector.x;
  y -= vector.y;
  z -= vector.z;
  return *this;
}

oglm::vec4::vec4(float x, float y, float z, float w) {
  this->x = x;
  this->y = y;
  this->z = z;
  this->w = w;
}

oglm::vec4::vec4(vec3 vector) {
  x = vector.x;
  y = vector.y;
  z = vector.z;
  w = 0.0f;
}

oglm::mat3::mat3(vec3 vec0, vec3 vec1, vec3 vec2) {
  this->columns[0] = vec0;
  this->columns[1] = vec1;
  this->columns[2] = vec2;
}

oglm::mat3::mat3(float value) {
  columns[0] = vec3(value, 0.0f, 0.0f);
  columns[1] = vec3(0.0f, value, 0.0f);
  columns[2] = vec3(0.0f, 0.0f, value);
}

oglm::mat3::mat3(mat4 matrix) {
  columns[0] = vec3(matrix.columns[0]);
  columns[1] = vec3(matrix.columns[1]);
  columns[2] = vec3(matrix.columns[2]);
}

oglm::mat3 oglm::mat3::operator * (float f) const {
  mat3 result = mat3(columns[0]*f, columns[1]*f, columns[2]*f);
  return result;
}

oglm::mat4::mat4(vec4 column0, vec4 column1, vec4 column2, vec4 column3) {
  this->columns[0] = column0;
  this->columns[1] = column1;
  this->columns[2] = column2;
  this->columns[3] = column3;
}

oglm::mat4::mat4(float value) {
  columns[0] = vec4(value, 0.0f, 0.0f, 0.0f);
  columns[1] = vec4(0.0f, value, 0.0f, 0.0f);
  columns[2] = vec4(0.0f, 0.0f, value, 0.0f);
  columns[3] = vec4(0.0f, 0.0f, 0.0f, value);
}

oglm::mat4::mat4(mat3 matrix) {
  columns[0] = vec4(matrix.columns[0]);
  columns[1] = vec4(matrix.columns[1]);
  columns[2] = vec4(matrix.columns[2]);
  columns[3] = vec4(0.0f, 0.0f, 0.0f, 1.0f);
}

oglm::mat4 oglm::mat4::operator * (mat4 matrix) const {
  mat4 result;
  for(unsigned int i = 0; i < 4; i++) {
    result.columns[i].x = (this->columns[0].x * matrix.columns[i].x) + (this->columns[1].x * matrix.columns[i].y) +
                          (this->columns[2].x * matrix.columns[i].z) + (this->columns[3].x * matrix.columns[i].w);
    result.columns[i].y = (this->columns[0].y * matrix.columns[i].x) + (this->columns[1].y * matrix.columns[i].y) +
                          (this->columns[2].y * matrix.columns[i].z) + (this->columns[3].y * matrix.columns[i].w);
    result.columns[i].z = (this->columns[0].z * matrix.columns[i].x) + (this->columns[1].z * matrix.columns[i].y) +
                          (this->columns[2].z * matrix.columns[i].z) + (this->columns[3].z * matrix.columns[i].w);
    result.columns[i].w = (this->columns[0].w * matrix.columns[i].x) + (this->columns[1].w * matrix.columns[i].y) +
                          (this->columns[2].w * matrix.columns[i].z) + (this->columns[3].w * matrix.columns[i].w);
  }
  return result;
}