#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 outColor;
layout(location = 3) in vec4 worldPosition; // we can use this

void main() {
    // Create a smooth gradient along X without blinking
    float gradient = 0.5 + 0.5 * sin(worldPosition.x * 5.0);
    
    // Apply green color with the gradient
    vec3 color = vec3(0.0, 1.0, 0.0) * gradient;
    
    outColor = vec4(color, 1.0);
}
