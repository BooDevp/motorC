#version 330 core

in vec2 vUV;
in vec3 vColor; // Ya no lo necesitaremos tanto, pero lo dejamos por si acaso
out vec4 FragColor;

// --- NUEVAS TEXTURAS ---
uniform sampler2D u_mask;    // Mapa de calor (blanco y negro)
uniform sampler2D u_texture; // Mapa de color (tu pintura de tabaco/papel)

uniform float iTime;
uniform float intensidadBrasa;
uniform float velocidadLatido;
uniform vec3 colorBrasa;

void main() {
    // Corregimos la orientación de la textura (flip Y)
    vec2 uv = vec2(vUV.x, 1.0 - vUV.y);

    // 1. LEEMOS EL COLOR DEL CIGARRO (La textura que pintaste)
    vec3 colorTextura = texture(u_texture, uv).rgb;

    // 2. LEEMOS LA MÁSCARA DE CALOR
    float maskRaw = texture(u_mask, uv).r;
    float maskExplosiva = pow(maskRaw, 2.5); 

    // 3. LÓGICA DE LA BRASA DINÁMICA
    float latido = 1.0 + 0.8 * sin(iTime * velocidadLatido);
    vec3 brasaDinamica = colorBrasa * latido * intensidadBrasa;

    // 4. MEZCLA FINAL: 
    // Ahora usamos 'colorTextura' en lugar de 'vColor'
    vec3 finalRGB = mix(colorTextura, brasaDinamica, maskExplosiva);
    
    // Añadimos el "glow" extra en las zonas más blancas
    finalRGB += colorBrasa * maskExplosiva * 0.5 * latido;

    FragColor = vec4(finalRGB, 1.0);
}