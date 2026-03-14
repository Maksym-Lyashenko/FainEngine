#version 450

layout(set = 0, binding = 0) uniform PerFrameData
{
    mat4 uModel;
    mat4 uView;
    mat4 uProj;
    vec4 uCameraPos;
};

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec3 outWorldPos;
layout(location = 1) out vec3 outWorldNormal;
layout(location = 2) out vec2 outUV;

void main()
{
    vec4 worldPos = uModel * vec4(inPosition, 1.0);
    mat3 normalMatrix = mat3(transpose(inverse(uModel)));

    outWorldPos = worldPos.xyz;
    outWorldNormal = normalize(normalMatrix * inNormal);
    outUV = inUV;

    gl_Position = uProj * uView * worldPos;
}