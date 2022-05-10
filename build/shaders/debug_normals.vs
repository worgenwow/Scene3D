#version 330 core
layout (location = 0) in vec3 aPos;   // the position variable has attribute position 0
layout (location = 1) in vec3 aNormal;

layout (std140) uniform Matrices {
    mat4 projection;
    mat4 view;
};

out VS_OUT {
  vec3 normal;
} vs_out;

uniform mat4 model;
uniform mat3 normalMatrix;

void main() {
    gl_Position = view * model * vec4(aPos, 1.0);
    vs_out.normal = normalize(normalMatrix * aNormal);
}