#version 450

// Forzamos el alineamiento estándar de 16 bytes
layout(std140, set = 0, binding = 0) uniform Data {
    mat4 mvp;
};

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inColor;
layout(location = 0) out vec3 fragColor;

void main() {
    // Si la matriz identidad falla, esto la ignora pero mantiene el binding activo
    // gl_Position = mvp * vec4(inPos, 1.0); 
    
    // PRUEBA DE FUERZA BRUTA: 
    // Dibujamos directamente y solo usamos un valor de la matriz para el color
    // Si ves el cuadrado de colores, es que el problema es la multiplicación de la matriz
    gl_Position = vec4(inPos.xy, 0.5, 1.0);
    fragColor = inColor;
}