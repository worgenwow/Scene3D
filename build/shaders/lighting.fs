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

struct Spotlight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

uniform vec3 viewPos;
uniform Spotlight spotlight;
uniform DirLight dirLight;
uniform PointLight pointLight;
uniform Material material;

vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 diffuseColor, vec3 specularColor);
vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 diffuseColor, vec3 specularColor);
vec3 calcSpotlight(Spotlight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 diffuseColor, vec3 specularColor);

void main() {
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos-FragPos);
    
    vec3 diffuseColor = texture(material.texture_diffuse1, TexCoords).rgb;
    vec3 specularColor = texture(material.texture_specular1, TexCoords).rgb;

    vec3 result = calcDirLight(dirLight, norm, viewDir, diffuseColor, specularColor);
    result += calcPointLight(pointLight, norm, FragPos, viewDir, diffuseColor, specularColor);
    result += calcSpotlight(spotlight, norm, FragPos, viewDir, diffuseColor, specularColor);

    FragColor = vec4(result, 1.0);
}

vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 diffuseColor, vec3 specularColor) {
    vec3 lightDir = normalize(-light.direction);
    // calculate diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);

    // calculate specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.specularExponent);

    // get results
    vec3 ambient = diffuseColor * light.ambient;
    vec3 diffuse = diffuseColor * diff * light.diffuse;
    vec3 specular = specularColor * spec * light.specular;

    // combine results
    return (ambient + diffuse + specular);
}

vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 diffuseColor, vec3 specularColor) {
    vec3 lightDir = normalize(light.position - fragPos);

    // calculate diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);

    // calculate specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.specularExponent);

    // dim light based on distance
    float distance = length(light.position - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    // get results
    vec3 ambient = diffuseColor * light.ambient;
    vec3 diffuse = diffuseColor * diff * light.diffuse;
    vec3 specular = specularColor * spec * light.specular;
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    // combine results
    return (ambient + diffuse + specular);
}

vec3 calcSpotlight(Spotlight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 diffuseColor, vec3 specularColor) {
    vec3 lightDir = normalize(light.position - fragPos);

    // calculate diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);

    // calculate specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.specularExponent);

    // dim light based on distance
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    // dim light as it gets outside of spotlight
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    // get results
    vec3 ambient = diffuseColor * light.ambient;
    vec3 diffuse = diffuseColor * diff * light.diffuse;
    vec3 specular = specularColor * spec * light.specular;
    ambient  *= intensity * attenuation;
    diffuse  *= intensity * attenuation;
    specular *= intensity * attenuation;
    // combine results
    return (ambient + diffuse + specular);
}