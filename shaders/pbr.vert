#version 450
layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec3 vColor;
layout (location = 3) in vec2 vTexCoord;

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec2 texCoord;
layout (location = 2) out vec3 normalWorld;
layout (location = 3) out vec3 posWorld;

layout (push_constant) uniform constants
{
	vec4 pos;
	mat4 projView;
}camera;

layout (set = 0, binding = 1) uniform UniformBufferModel
{
	mat4 model;
}model;

layout(set = 1, binding = 1) uniform sampler2D normalTexure;

void main()
{
	gl_Position = camera.projView * model.model *vec4(vPosition, 1.0f);
    posWorld = vec3(model.model * vec4(vPosition, 1.0));

	vec3 normal = texture(normalTexure, vTexCoord).xyz;
    normalWorld = mat3(transpose(inverse(model.model))) * normal;

	outColor = vColor;

	texCoord = vTexCoord;
}