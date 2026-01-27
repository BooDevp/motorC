#version 330 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D screenTexture;
uniform float iTime;
uniform vec2 iResolution;

// --- Parámetros de Control (puedes convertirlos a uniforms si quieres) ---
const float brightness = 1.02;
const float contrast   = 1.08;
const float saturation = 1.10;
const float gamma      = 1.05;
const float curvature  = 0.14;
const float corner_soften = 0.06;
const float vignette   = 0.38;
const float scanline_strength = 0.55; // Bajado un poco para que no oscurezca tanto
const float scanline_density  = 1.15;
const float mask_strength = 0.22;
const float mask_scale = 1.0;
const float chroma_offset_px = 1.6;
const float jitter_px = 1.8;

// --- Funciones de Ruido Optimizado ---
float hash11(float n) {
    return fract(sin(n) * 43758.5453123);
}

float hash12(vec2 p) {
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453123);
}

vec2 crt_curve(vec2 uv, float k) {
    vec2 cc = uv * 2.0 - 1.0;
    vec2 d  = cc * cc;
    cc *= 1.0 + k * vec2(d.y, d.x);
    return cc * 0.5 + 0.5;
}

vec3 apply_color_controls(vec3 c) {
    c *= brightness;
    c = mix(vec3(0.5), c, contrast);
    float l = dot(c, vec3(0.2126, 0.7152, 0.0722));
    c = mix(vec3(l), c, saturation);
    c = pow(max(c, vec3(0.0)), vec3(1.0 / max(gamma, 0.0001)));
    return c;
}

void main() {
    vec2 px = 1.0 / iResolution;
    float t = iTime;
    vec2 uv = TexCoords;

    // --- VHS Jitter (Desplazamiento horizontal por línea) ---
    float y_line = floor(uv.y * iResolution.y);
    float jitter = (hash11(y_line + floor(t * 15.0) * 37.0) - 0.5) * 2.0;
    uv.x += jitter * (jitter_px * px.x);

    // --- Curvatura CRT ---
    vec2 cuv = crt_curve(uv, curvature);
    
    // Verificar si estamos fuera de la pantalla curva (bordes negros)
    float in_bounds = step(0.0, cuv.x) * step(cuv.x, 1.0) * step(0.0, cuv.y) * step(cuv.y, 1.0);

    // Esquinas redondeadas
    vec2 corner_d = abs(cuv - 0.5) * 2.0;
    float edge = max(corner_d.x, corner_d.y);
    float corner_mask = 1.0 - smoothstep(1.0 - corner_soften, 1.0, edge);

    // --- OPTIMIZACIÓN: Muestreo de Textura (Chrominance Split) ---
    // Hacemos el RGB split de una vez
    vec2 chroma = vec2(chroma_offset_px * px.x, 0.0);
    float r = texture(screenTexture, cuv + chroma).r;
    float g = texture(screenTexture, cuv).g;
    float b = texture(screenTexture, cuv - chroma).b;
    vec3 col = vec3(r, g, b);

    // --- Scanlines ---
    float scan = sin((cuv.y * iResolution.y) * 3.14159 * scanline_density);
    float scan_mul = 1.0 - scanline_strength * (0.5 - 0.5 * scan);
    col *= scan_mul;

    // --- Rolling Bar (La barra de interferencia que sube) ---
    float roll_y = fract(t * 0.1);
    float dist = abs(cuv.y - roll_y);
    float bar = smoothstep(0.15, 0.0, dist);
    col += bar * 0.12;

    // --- Ruido de Cinta (Tape Grain) ---
    float noise = hash12(gl_FragCoord.xy + vec2(t * 12.0, t * 90.0));
    col += (noise - 0.5) * 0.04;

    // --- Vignette ---
    vec2 v = cuv - 0.5;
    float vig = smoothstep(0.9, 0.2, dot(v, v) * 2.2);
    col *= mix(1.0 - vignette, 1.0, vig);

    // --- Shadow Mask (Triadas RGB de los fósforos) ---
    float x_mask = floor(gl_FragCoord.x / mask_scale);
    float m = mod(x_mask, 3.0);
    vec3 mask_col = vec3(1.0);
    if (m < 1.0) mask_col = vec3(1.1, 0.9, 0.9);
    else if (m < 2.0) mask_col = vec3(0.9, 1.1, 0.9);
    else mask_col = vec3(0.9, 0.9, 1.1);
    col *= mix(vec3(1.0), mask_col, mask_strength);

    // Post-procesado final de color
    col = apply_color_controls(col);

    // Aplicar bordes negros de la curvatura
    col *= in_bounds * corner_mask;

    FragColor = vec4(col, 1.0);
}