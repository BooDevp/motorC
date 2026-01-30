#version 330 core

in vec2 vUV;
in vec3 vColor;
out vec4 FragColor;

uniform sampler2D u_mask;
uniform float iTime;

uniform float intensidadBrasa;
uniform float velocidadLatido;
uniform vec3 colorBrasa;

void main() {
    // 1. Leemos la máscara
    float maskRaw = texture(u_mask, vec2(vUV.x, 1.0 - vUV.y)).r;

    // 2. LOGICA DE REALISMO:
    // Usamos pow(mask, 2.0) para que los grises oscuros casi no brillen 
    // y los blancos brillen mucho. Esto da un degradado mucho más natural.
    float maskExplosiva = pow(maskRaw, 2.5); 

    // 3. Color base (tabaco/papel)
    vec3 colorBase = vColor;
    if(length(colorBase) < 0.1) colorBase = vec3(0.7, 0.7, 0.7);

    // 4. Brasa dinámica
    // El latido ahora afecta más a las zonas muy blancas que a las grises
    float latido = 1.0 + 0.8 * sin(iTime * velocidadLatido);
    vec3 brasaDinamica = colorBrasa * latido * intensidadBrasa;

    // 5. MEZCLA FINAL ADITIVA:
    // mix() hace la transición, pero sumar un poco de colorBrasa extra 
    // en las zonas blancas (maskRaw) da ese efecto de "luz emitida".
    vec3 finalRGB = mix(colorBase, brasaDinamica, maskExplosiva);
    
    // Añadimos un pequeño "glow" extra solo donde es muy blanco
    finalRGB += colorBrasa * maskExplosiva * 0.5 * latido;

    FragColor = vec4(finalRGB, 1.0);
}