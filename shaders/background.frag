#version 450 core

layout (location = 0) in vec2 texFrag;

layout (location = 0) out vec4 colorFragOut;

layout(set = 1, binding = 0) uniform sampler2D textureSampler;

void main()
{
    colorFragOut = texture(textureSampler, texFrag);
}