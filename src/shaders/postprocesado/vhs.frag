#version 330 core
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D screenTexture;
uniform float iTime;
uniform vec2 iResolution;

const float jitter_px = 1.8;
const float chroma_offset_px = 1.6;

float hash11(float n) { return fract(sin(n) * 43758.5453123); }
float hash12(vec2 p) { return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453123); }

void main() {
    vec2 px = 1.0 / iResolution;
    vec2 uv = TexCoords;

    // Jitter horizontal
    float y_line = floor(uv.y * iResolution.y);
    float jitter = (hash11(y_line + floor(iTime * 15.0) * 37.0) - 0.5) * 2.0;
    uv.x += jitter * (jitter_px * px.x);

    // Separación de color (Chroma shift)
    vec2 chroma = vec2(chroma_offset_px * px.x, 0.0);
    float r = texture(screenTexture, uv + chroma).r;
    float g = texture(screenTexture, uv).g;
    float b = texture(screenTexture, uv - chroma).b;
    vec3 col = vec3(r, g, b);

    // Barra de interferencia (Rolling bar)
    float roll = smoothstep(0.15, 0.0, abs(uv.y - fract(iTime * 0.1)));
    col += roll * 0.1;

    // Ruido de grano
    col += (hash12(gl_FragCoord.xy + iTime) - 0.5) * 0.05;

    FragColor = vec4(col, 1.0);
}