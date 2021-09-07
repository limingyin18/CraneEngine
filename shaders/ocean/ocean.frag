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

//layout(set = 1, binding = 0) uniform sampler2D textureSampler;


void main()
{
    float Kdiffuse = 0.91f;

    vec3 lightDir = normalize(-sceneData.sunlightDirection.xyz);

    float reflectivity;
    float nSnell = 1.34f;
    float costhetai = abs(dot(lightDir, normalFrag));
    float thetai = acos(costhetai);
    float sinthetat = sin(thetai)/nSnell;
    float thetat = asin(sinthetat);
    if(thetai == 0.0)
    {
        reflectivity = (nSnell - 1)/(nSnell + 1);
        reflectivity = reflectivity * reflectivity;
    }
    else
    {
        float fs = sin(thetat - thetai) / sin(thetat + thetai);
        float ts = tan(thetat - thetai) / tan(thetat + thetai);
        reflectivity = 0.5 * ( fs*fs + ts*ts );
    }


    float ambient = 0.1f;

    float specular = 0.0f;
    float diffuse = max(dot(normalFrag, lightDir), 0.0);
    if(diffuse > 0.01f)
    {
        vec3 viewDir = normalize(camera.pos.xyz - posFrag);
        vec3 reflectDir = reflect(-lightDir, normalFrag);
        specular = pow(max(dot(viewDir, reflectDir), 0.0), 32) * 0.01f;
    }

    colorFragOut = vec4(sceneData.ambientColor.xyz + 
        reflectivity * vec3(0.69f, 0.84f, 1.f) + (1-reflectivity) * vec3(0.0f, 0.2f, 0.3f) +
        specular * sceneData.sunlightColor.xyz, 1.0f);
}