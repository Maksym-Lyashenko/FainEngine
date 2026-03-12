#version 450

layout(location = 0) in vec3 inColor;
layout(location = 0) out vec4 outColor;

layout(constant_id = 0) const uint IS_WIREFRAME = 0u;

void main()
{
    if (IS_WIREFRAME != 0u)
    {
        outColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
    else
    {
        outColor = vec4(inColor, 1.0);
    }
}