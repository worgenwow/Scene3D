#version 330 core
out vec4 FragColor;

struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_diffuse2;
    sampler2D texture_diffuse3;
    sampler2D texture_specular1;
    sampler2D texture_specular2;

    float specularExponent;
};

uniform Material material;

in vec2 TexCoords;

void main() {
    vec4 tex = texture(material.texture_diffuse1, TexCoords);
    if(tex.a < 0.1) {
        discard;
    }
    
    FragColor = tex;
}