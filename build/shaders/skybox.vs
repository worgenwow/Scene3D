#version 330 core
layout (location = 0) in vec3 aPos;

layout (std140) uniform Matrices {
    mat4 projection;
    mat4 view;
};

uniform mat4 skyboxView;

out vec3 TexCoords;
void main() {
  TexCoords = aPos;
  vec4 position = projection * skyboxView * vec4(aPos, 1.0);
  gl_Position = position.xyww;
}