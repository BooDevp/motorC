#version 450 core
in vec3 ourColor;
out vec4 FragColor;

uniform float u_time; // Variable que controlaremos desde C

void main() {
    // Mezclamos el color del vértice con el tiempo para que parpadee
    float brightness = (sin(u_time) * 0.5) + 0.5;
    FragColor = vec4(ourColor * brightness, 1.0);
}