#version 450 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 colorVert;
layout (location = 3) in vec2 tex;

layout (location = 0) out vec3 colorFrag;
layout (location = 1) out vec2 texFrag;
layout (location = 2) out vec3 normalFrag;
layout (location = 3) out vec3 posFrag;

layout (set = 1, binding = 0) coherent buffer block0
{
    vec2 h[];
};
layout(set = 1, binding = 5) uniform sampler2D heightTex;

layout (set = 1, binding = 1) coherent buffer block1
{
    vec2 normalX[];
};

layout (set = 1, binding = 2) coherent buffer block2
{
    vec2 normalZ[];
};

layout (set = 1, binding = 3) coherent buffer block3
{
    vec2 dx[];
};

layout (set = 1, binding = 4) coherent buffer block4
{
    vec2 dz[];
};

layout (set = 0, binding = 2) uniform bufferLambda 
{
    float lambda;
};

layout (push_constant) uniform constants
{
	vec4 pos;
	mat4 projView;
}camera;

layout (set = 0, binding = 1) uniform UniformBufferModel
{
	mat4 model;
}model;

void main()
{
    vec2 texInv = vec2(1.0f - tex.y, tex.x);
    vec4 height = texture(heightTex, texInv);

    vec3 pos = position + vec3(-lambda*dx[gl_VertexIndex].x, h[gl_VertexIndex].x, -lambda*dz[gl_VertexIndex].x);
    gl_Position =  camera.projView * model.model* vec4(pos.x, pos.y, pos.z, 1.0);
    posFrag = vec3(model.model* vec4(pos.x, pos.y, pos.z, 1.0));
    colorFrag = colorVert;
    texFrag = tex;
    normalFrag = vec3(normalX[gl_VertexIndex].x, 1.0f, normalZ[gl_VertexIndex].x);
    normalFrag = normalize(normalFrag);
}