#version 450 core

layout (location = 0) in vec3 colorFrag;
layout (location = 1) in vec2 texFrag;
layout (location = 2) in vec3 normalFrag;
layout (location = 3) in vec3 posFrag;

layout (location = 0) out vec4 colorFragOut;

layout (push_constant) uniform constants
{
	vec4 pos;
	mat4 projView;
}camera;

layout(set = 0, binding = 0) uniform  SceneData{   
    vec4 fogColor;
	vec4 fogDistances;
	vec4 ambientColor;
	vec4 sunlightDirection;
	vec4 sunlightColor;
} sceneData;

layout(set = 1, binding = 0) uniform sampler2D textureSampler;

void main()
{
    vec4 baseColor = texture(textureSampler, texFrag);

    // ambient
    vec4 ambient = sceneData.ambientColor * baseColor;

    // diff
    vec3 norm = normalize(normalFrag);
    vec3 lightDir = normalize(-sceneData.sunlightDirection.rgb);
    float diff = max(dot(norm, lightDir), 0.0);
    vec4 diffuse = sceneData.sunlightColor * diff * baseColor;

    // specular
    vec3 viewDir = normalize(camera.pos.xyz - posFrag);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec4 specular = sceneData.sunlightColor * spec * baseColor;

    // result
    colorFragOut = vec4(colorFrag, 1.0f) * (ambient + diffuse + specular);
}