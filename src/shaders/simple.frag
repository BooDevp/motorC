#version 330 core
in vec3 vNormal;
in vec3 vColor;
out vec4 outColor;
void main() {
    vec3 n = normalize(vNormal);
    
    // Luz desde la esquina (estilo Blender por defecto)
    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.7));
    
    // Difuso con un toque de 'envuelto' para que no haya zonas negras puras por sombra
    float diff = max(dot(n, lightDir), 0.0);
    float lighting = diff * 0.7 + 0.3; 
    vec3 result = vColor * lighting;
    
    // Corrección Gamma para que los medios tonos sean iguales a Blender
    vec3 finalColor = pow(result, vec3(1.0/2.2));    
    outColor = vec4(finalColor, 1.0);
}