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
    vec3 norm = normalize(normalFrag);
    vec3 lightDir = normalize(-sceneData.sunlightDirection.xyz);
    vec3 lightColor = sceneData.sunlightColor.xyz;
    float diffuse = max(dot(norm, lightDir), 0.0);

    float reflectivity;
    float nSnell = 1.34f;
    float costhetai = abs(dot(lightDir, norm));
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
    if(diffuse > 0.01f)
    {
        vec3 viewDir = normalize(camera.pos.xyz - posFrag);
        vec3 reflectDir = reflect(-lightDir, norm);
        specular = pow(max(dot(viewDir, reflectDir), 0.0), 32) * 0.01f;
    }

    colorFragOut = vec4((ambient + reflectivity + specular) * lightColor, 1.0f);
    //colorFragOut = vec4((ambient + reflectivity) * lightColor, 1.0f);
}