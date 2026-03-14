#version 450

layout(set = 0, binding = 1) uniform samplerCube uEnvironment;

layout(location = 0) in vec3 outDir;
layout(location = 0) out vec4 outColor;

void main()
{
    vec3 dir = normalize(outDir);
    vec3 color = texture(uEnvironment, dir).rgb;
    outColor = vec4(color, 1.0);
}