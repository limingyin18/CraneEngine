#version 450 core
#extension GL_KHR_vulkan_glsl:enable

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 colorVert;
layout (location = 3) in vec2 tex;

layout (location = 0) out vec3 colorFrag;
layout (location = 1) out vec2 texFrag;
layout (location = 2) out vec3 normalFrag;
layout (location = 3) out vec3 posFrag;

layout(set = 1, binding = 0) uniform sampler2D heightTex;

layout(set = 1, binding = 1) uniform sampler2D normalXTex;

layout(set = 1, binding = 2) uniform sampler2D normalZTex;

layout(set = 1, binding = 3) uniform sampler2D dxTex;

layout(set = 1, binding = 4) uniform sampler2D dzTex;

layout (set = 1, binding = 5) uniform bufferLambda 
{
    float lambda;
};

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

    //vec2 texInv = vec2(1.0f - tex.y, tex.x);
    vec2 texInv = vec2(tex.x, tex.y);
    vec4 height = texture(heightTex, texInv);
    vec4 normalX = texture(normalXTex, texInv);
    vec4 normalZ = texture(normalZTex, texInv);
    vec4 dx = texture(dxTex, texInv);
    vec4 dz = texture(dzTex, texInv);

    vec3 pos = position + vec3(-lambda*dx.x, height.x, -lambda*dz.x);
    //vec3 pos = position + vec3(0.f, height.x, 0.f);
    gl_Position =  camera.projView * model* vec4(pos.x, pos.y, pos.z, 1.0);
    posFrag = vec3(model* vec4(pos.x, pos.y, pos.z, 1.0));
    colorFrag = colorVert;
    texFrag = tex;
    normalFrag = vec3(normalX.x, 1.0f, normalZ.x);
    normalFrag = normalize(mat3(transpose(inverse(model))) *(normalFrag));
}