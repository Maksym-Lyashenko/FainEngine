#version 450

layout(set = 0, binding = 0) uniform PerFrameData
{
    mat4 uModel;
    mat4 uView;
    mat4 uProj;
    vec4 uCameraPos;
};

layout(location = 0) out vec3 outDir;

vec3 kPositions[36] = vec3[](
    vec3(-1, -1, -1), vec3( 1, -1, -1), vec3( 1,  1, -1),
    vec3( 1,  1, -1), vec3(-1,  1, -1), vec3(-1, -1, -1),

    vec3(-1, -1,  1), vec3( 1, -1,  1), vec3( 1,  1,  1),
    vec3( 1,  1,  1), vec3(-1,  1,  1), vec3(-1, -1,  1),

    vec3(-1,  1,  1), vec3(-1,  1, -1), vec3(-1, -1, -1),
    vec3(-1, -1, -1), vec3(-1, -1,  1), vec3(-1,  1,  1),

    vec3( 1,  1,  1), vec3( 1,  1, -1), vec3( 1, -1, -1),
    vec3( 1, -1, -1), vec3( 1, -1,  1), vec3( 1,  1,  1),

    vec3(-1, -1, -1), vec3( 1, -1, -1), vec3( 1, -1,  1),
    vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, -1, -1),

    vec3(-1,  1, -1), vec3( 1,  1, -1), vec3( 1,  1,  1),
    vec3( 1,  1,  1), vec3(-1,  1,  1), vec3(-1,  1, -1)
);

void main()
{
    vec3 pos = kPositions[gl_VertexIndex];
    outDir = pos;

    mat4 viewNoTranslation = mat4(mat3(uView));
    vec4 clipPos = uProj * viewNoTranslation * vec4(pos, 1.0);
    gl_Position = clipPos.xyww;
}