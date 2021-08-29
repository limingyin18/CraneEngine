#version 450

layout (location = 0) in vec3 inColor;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in vec3 normalWorld;
layout (location = 3) in vec3 posWorld;

layout (location = 0) out vec4 outFragColor;

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

layout(set = 1, binding = 0) uniform sampler2D albedoTexture;

layout(set = 1, binding = 2) uniform sampler2D metallicTexture;

// brdf auxiliary functions.
float NormalDistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 FresnelSchlick(float cosTheta, vec3 F0);

const float PI = 3.14f;

void main()
{
	// 表面
	vec3 albedo = texture(albedoTexture, texCoord).rgb;
	// 金属度
	float metallic = texture(metallicTexture, texCoord).b;
	// 粗糙度
	float roughness = texture(metallicTexture, texCoord).g;

	// 视线方向
    vec3 viewDir = normalize(camera.pos.xyz - posWorld);
	// 平行光方向
	vec3 lightDir = normalize(-sceneData.sunlightDirection.xyz);
	// 半程向量
	vec3 halfwayDir = normalize(lightDir + viewDir);

	// 基础反射率
	vec3 F0 = vec3(0.04);
	F0 = mix(F0, albedo, metallic);
	// 镜面反射率
	vec3 kSpecular = FresnelSchlick(max(dot(halfwayDir, viewDir), 0), F0);
	// 法线分布
	float normalFactor = NormalDistributionGGX(normalWorld, halfwayDir, roughness);
	// 几何分布
	float geometryFactor = GeometrySmith(normalWorld, viewDir, lightDir, roughness);
	// 漫反射率
	vec3 kDiffuse = vec3(1.0f) - kSpecular;
	kDiffuse *= (1.0f - metallic);

	// 漫反射
	vec3 diffuse = kDiffuse*albedo/PI;
	// 镜面反射
	vec3 specular = normalFactor * kSpecular * geometryFactor / 
		(4.0f * max(dot(viewDir, normalWorld), 0.f) * max(dot(lightDir, normalWorld), 0.0f) + 0.0001f);

	// 颜色
	vec3 color = (diffuse + specular)*sceneData.sunlightColor.rgb*
		max(dot(lightDir, normalWorld), 0.0f);
	color = color / (color + vec3(1.0)); // 色调映射
	color = pow(color, vec3(1.0/2.2)); 	 // 伽马校正
	outFragColor = vec4(color, 1.0f);
}

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
	return  F0 + (1.0f - F0) * pow(1.0f - cosTheta, 5.0);
}

float NormalDistributionGGX(vec3 N, vec3 H, float a)
{
	float a2     = a * a;
	float NdotH  = max(dot(N, H), 0.f);
	float NdotH2 = NdotH * NdotH;

	float nom    = a2;
	float denom  = NdotH2*(a2 - 1.f)+1.f;
	denom 		 = PI * denom * denom;

	return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float k)
{
    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    return nom / denom;
}
  
float GeometrySmith(vec3 N, vec3 V, vec3 L, float k)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, k); // 视线方向的几何遮挡
    float ggx2 = GeometrySchlickGGX(NdotL, k); // 光线方向的几何阴影
    return ggx1 * ggx2;
}