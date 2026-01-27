#version 330 core

in vec3 vNormal;
in vec3 vColor;

out vec4 FragColor;

void main() {
    // 1. Normalizamos la normal que viene del modelo
    vec3 n = normalize(vNormal);
    
    // 2. Luz direccional: viene desde arriba y un poco de lado
    vec3 lightDir = normalize(vec3(0.3, 1.0, 0.4));
    
    // 3. Cálculo de incidencia (Lambert)
    float diff = max(dot(n, lightDir), 0.0);
    
    // 4. Luz ambiental equilibrada
    float lighting = 0.3 + (diff * 0.7);
    
    // 5. Aplicamos color ROJO fijo
    vec3 redColor = vec3(1.0, 0.0, 0.0);
    vec3 finalColor = redColor * lighting;
    
    FragColor = vec4(finalColor, 1.0);
}
