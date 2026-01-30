#version 330 core

// Atributos de entrada (Exactamente como los tenías tú)
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;   // Las UVs están en el hueco 2
layout(location = 3) in vec3 aColor;

// Uniforms (Usamos tu uMVP)
uniform mat4 uMVP; 

// Variables de salida para el Fragment Shader
out vec3 vNormal;
out vec3 vColor;
out vec2 vUV;      

void main() {
    // Usamos tu matriz combinada uMVP
    gl_Position = uMVP * vec4(aPos, 1.0);
    
    // Pasamos los datos al fragment shader
    vNormal = aNormal;
    vColor = aColor;
    vUV = aUV; 
}