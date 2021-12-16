#version 450 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 colorVert;
layout (location = 3) in vec2 tex;

layout (location = 1) out vec2 texFrag;

layout (push_constant) uniform constants
{
	vec4 pos;
	mat4 projView;
}camera;

void main()
{
	gl_Position = vec4(position, 1.0f);
    texFrag = tex;
}