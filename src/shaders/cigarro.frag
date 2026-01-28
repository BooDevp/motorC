#version 330 core

in vec2 vUV;
in vec3 vColor;
out vec4 FragColor;

uniform sampler2D u_mask;
uniform float iTime;

// Parámetros dinámicos configurables desde C
uniform float intensidadBrasa;
uniform float velocidadLatido;
uniform vec3 colorBrasa;

void main() {
    // 1. Leemos la máscara (la punta blanca que pintaste)
    // Usamos 1.0 - vUV.y por si Blender exportó las UVs invertidas
    float mask = texture(u_mask, vec2(vUV.x, 1.0 - vUV.y)).r;

    // 2. Color base del cigarro (usamos vColor que viene de Blender)
    vec3 colorBase = vColor;
    
    // Si vColor llega negro por error, le damos un gris claro por defecto
    if(length(colorBase) < 0.1) colorBase = vec3(0.7, 0.7, 0.7);

    // 3. Efecto de Brasa (Naranja intenso con latido) - AHORA CONFIGURABLE
    float latido = 1.2 + 0.6 * sin(iTime * velocidadLatido);
    vec3 colorBrasaFinal = colorBrasa * latido * intensidadBrasa;

    // 4. Mezcla final: Donde mask es 1.0 (blanco) se ve la brasa
    vec3 finalRGB = mix(colorBase, colorBrasaFinal, mask);

    FragColor = vec4(finalRGB, 1.0);
}