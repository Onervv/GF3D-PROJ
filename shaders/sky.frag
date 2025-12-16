#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec4 color;
    float time;
} ubo;

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec4 colorMod;
layout(location = 2) in vec3 localPos;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 dir = normalize(localPos);

    vec3 colorBottom = vec3(0.0, 1.0, 1.0);
    vec3 colorTop = vec3(1.0, 0.0, 1.0);
    vec3 colorBg = vec3(0.02, 0.02, 0.05);

    float heightFactor = smoothstep(-0.2, 0.8, dir.y);
    vec3 gradientGrid = mix(colorBottom, colorTop, heightFactor);

    vec3 absDir = abs(dir);
    float maxDir = max(absDir.x, max(absDir.y, absDir.z));
    vec2 uv;

    if (absDir.x >= maxDir) uv = dir.yz / dir.x;
    else if (absDir.y >= maxDir) uv = dir.xz / dir.y;
    else uv = dir.xy / dir.z;

    float gridSize = 5.0;
    vec2 gridPos = fract(uv * gridSize);
    
    float distToLine = min(gridPos.x, 1.0 - gridPos.x);
    distToLine = min(distToLine, min(gridPos.y, 1.0 - gridPos.y));

    float glow = 0.02 / (distToLine + 0.05);
    float core = 0.005 / (distToLine + 0.005);
    
    vec3 finalColor = colorBg;
    finalColor += gradientGrid * glow * 1.5;
    finalColor += vec3(1.0) * core * 0.8;

    float horizonFade = smoothstep(-0.1, 0.1, dir.y);
    finalColor *= (0.4 + 0.6 * horizonFade);

    outColor = vec4(finalColor, 1.0);
}