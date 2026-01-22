#version 450
layout(location = 0) in vec3 fragColor; // Entrada desde el Vertex Shader
layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(fragColor, 1.0);
}