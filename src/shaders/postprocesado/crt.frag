#version 330 core
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D screenTexture;
uniform vec2 iResolution;

const float curvature = 0.05;
const float corner_soften = 0.06;

const float scanline_strength = 0.40;
const float scanline_density = 1.15;
const float mask_strength = 0.20;

vec2 crt_curve(vec2 uv, float k) {
    vec2 cc = uv * 2.0 - 1.0;
    vec2 d = cc * cc;
    cc *= 1.0 + k * vec2(d.y, d.x);
    return cc * 0.5 + 0.5;
}

void main() {
    vec2 cuv = crt_curve(TexCoords, curvature);
    
    // Bordes y esquinas
    float in_bounds = step(0.0, cuv.x) * step(cuv.x, 1.0) * step(0.0, cuv.y) * step(cuv.y, 1.0);
    vec2 corner_d = abs(cuv - 0.5) * 2.0;
    float corner_mask = 1.0 - smoothstep(1.0 - corner_soften, 1.0, max(corner_d.x, corner_d.y));

    vec3 col = texture(screenTexture, cuv).rgb;

    // Scanlines (líneas horizontales)
    float scan = sin((cuv.y * iResolution.y) * 3.14159 * scanline_density);
    col *= 1.0 - scanline_strength * (0.5 - 0.5 * scan);

    // Máscara de fósforo (puntos RGB)
    float m = mod(floor(gl_FragCoord.x), 3.0);
    vec3 mask = (m < 1.0) ? vec3(1.1, 0.9, 0.9) : ((m < 2.0) ? vec3(0.9, 1.1, 0.9) : vec3(0.9, 0.9, 1.1));
    col *= mix(vec3(1.0), mask, mask_strength);

    FragColor = vec4(col * in_bounds * corner_mask, 1.0);
}