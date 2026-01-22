#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor; // Nueva entrada
out vec3 ourColor; 

void main() {
    gl_Position = vec4(aPos.x + sin(gl_InstanceID), aPos.y, aPos.z, 1.0);
    ourColor = aColor; // Pasamos el color al fragment shader
}