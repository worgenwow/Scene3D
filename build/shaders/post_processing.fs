#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

const float offset = 1.0 / 300.0;  

void main() {
  vec2 offsets[9] = vec2[](
    vec2(-offset,  offset), // top-left
    vec2( 0.0f,    offset), // top-center
    vec2( offset,  offset), // top-right
    vec2(-offset,  0.0f),   // center-left
    vec2( 0.0f,    0.0f),   // center-center
    vec2( offset,  0.0f),   // center-right
    vec2(-offset, -offset), // bottom-left
    vec2( 0.0f,   -offset), // bottom-center
    vec2( offset, -offset)  // bottom-right    
  );

  float sharpenKernel[9] = float[](
    1.0 / 16, 2.0 / 16, 1.0 / 16,
    2.0 / 16, 4.0 / 16, 2.0 / 16,
    1.0 / 16, 2.0 / 16, 1.0 / 16  
  );
  
  vec3 sampleTex[9];
  for(int i = 0; i < 9; i++)
  {
    sampleTex[i] = vec3(texture(screenTexture, TexCoords.st + offsets[i]));
  }
  vec3 col = vec3(0.0);
  for(int i = 0; i < 9; i++)
    col += sampleTex[i] * sharpenKernel[i];
  
  // vec3 finalColor = vec3(0,(col.r+col.g+col.b)/3, 0);
  vec3 finalColor = vec3(texture(screenTexture, TexCoords));
  // vec3 finalColor = col;

  // add gamma correction
  float gamma = 2.2;
  FragColor = vec4(pow(finalColor, vec3(1.0/gamma)), 1.0);
}