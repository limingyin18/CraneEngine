#version 450 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 colorVert;
layout (location = 3) in vec2 tex;

layout (location = 0) out vec3 colorFrag;
layout (location = 1) out vec2 texFrag;
layout (location = 2) out vec3 normalFrag;
layout (location = 3) out vec3 posFrag;

layout (push_constant) uniform constants
{
	vec4 pos;
	mat4 projView;
}camera;

layout (set = 0, binding = 1) readonly buffer BufferModel
{
	mat4 bufferModel[];
};

layout (set = 0, binding = 2) readonly buffer BufferInstance
{
	uint bufferInstance[];
};

void main()
{
    mat4 model = bufferModel[bufferInstance[gl_InstanceIndex]]; 
	gl_Position = camera.projView * model *vec4(position, 1.0f);
    posFrag = vec3(model * vec4(position, 1.0));
    colorFrag = colorVert;
    texFrag = tex;
    normalFrag = mat3(transpose(inverse(model))) * normal;
}