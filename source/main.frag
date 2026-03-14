#version 450

layout(set = 0, binding = 0) uniform PerFrameData
{
    mat4 uModel;
    mat4 uView;
    mat4 uProj;
    vec4 uCameraPos;
};

layout(set = 0, binding = 1) uniform sampler2D uAlbedo;
layout(set = 0, binding = 2) uniform samplerCube uEnvironment;

layout(constant_id = 0) const bool isWireframe = false;

layout(location = 0) in vec3 outWorldPos;
layout(location = 1) in vec3 outWorldNormal;
layout(location = 2) in vec2 outUV;

layout(location = 0) out vec4 outColor;

void main()
{
    if (isWireframe)
    {
        outColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }

    vec3 N = normalize(outWorldNormal);
    vec3 V = normalize(uCameraPos.xyz - outWorldPos);
    vec3 R = reflect(-V, N);

    vec3 albedo = texture(uAlbedo, outUV).rgb;
    vec3 env = texture(uEnvironment, R).rgb;

    vec3 L = normalize(vec3(0.4, 1.0, 0.3));
    float ndotl = max(dot(N, L), 0.0);
    float ambient = 0.2;

    vec3 lit = albedo * (ambient + ndotl);
    vec3 finalColor = mix(lit, env, 0.35);

    outColor = vec4(finalColor, 1.0);
}