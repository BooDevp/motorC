#version 330 core

in vec3 vNormal;
in vec3 vColor;

out vec4 FragColor;

void main() {
    // 1. Normalizamos la normal que viene del modelo
    vec3 n = normalize(vNormal);
    
    // 2. Luz direccional: viene desde arriba y un poco de lado
    // Esto asegura que cualquier forma (esfera, personaje, etc) tenga sombras que definan su silueta
    vec3 lightDir = normalize(vec3(0.3, 1.0, 0.4));
    
    // 3. Cálculo de incidencia (Lambert)
    float diff = max(dot(n, lightDir), 0.0);
    
    // 4. Luz ambiental equilibrada
    // 0.3 de base + 0.7 de incidencia de luz para no quemar el color
    float lighting = 0.3 + (diff * 0.7);
    
    // 5. Aplicamos al color original de la malla
    vec3 finalColor = vColor * lighting;
    
    FragColor = vec4(finalColor, 1.0);
}