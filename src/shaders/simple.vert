#version 330 core

// Atributos de entrada (deben coincidir con tus glVertexAttribPointer en C)
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;   // Nueva: Coordenadas de textura
layout(location = 3) in vec3 aColor;

// Uniforms
uniform mat4 uMVP; // Asegúrate de que se llame igual que en tu C

// Variables de salida para el Fragment Shader
out vec3 vNormal;
out vec3 vColor;
out vec2 vUV;      // Nueva: Pasamos las UVs al fragment

void main() {
    gl_Position = uMVP * vec4(aPos, 1.0);
    
    // Pasamos los datos al fragment shader
    vNormal = aNormal;
    vColor = aColor;
    vUV = aUV; 
}