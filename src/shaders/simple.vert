#version 450
layout(set = 0, binding = 0) uniform Data { mat4 mvp; };
layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inColor;
layout(location = 0) out vec3 fragColor;

void main() {
    // IMPORTANTE: En Vulkan, si no ves nada, prueba a no invertir la Y primero.
    gl_Position = mvp * vec4(inPos, 1.0);
    fragColor = inColor;
}