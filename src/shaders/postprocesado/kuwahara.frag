#version 330 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D screenTexture;
uniform vec2 iResolution;
uniform int radius = 4; // ¡Cuidado! Valores altos (>6) matarán los FPS

void main() {
    vec2 uv = TexCoords;
    vec2 src_size = 1.0 / iResolution;

    // Medias (m) y Varianzas (s) para los 4 cuadrantes
    vec3 m[4];
    vec3 s[4];
    for (int k = 0; k < 4; ++k) {
        m[k] = vec3(0.0);
        s[k] = vec3(0.0);
    }

    float n = float((radius + 1) * (radius + 1));

    // Bucle unificado: recorremos una vez y asignamos a cuadrantes
    // Esto es MUCHO más eficiente que 4 bucles separados
    for (int j = -radius; j <= 0; ++j) {
        for (int i = -radius; i <= 0; ++i) {
            // Superior Izquierda
            vec3 c0 = texture(screenTexture, uv + vec2(i, j) * src_size).rgb;
            m[0] += c0;
            s[0] += c0 * c0;

            // Superior Derecha
            vec3 c1 = texture(screenTexture, uv + vec2(-i, j) * src_size).rgb;
            m[1] += c1;
            s[1] += c1 * c1;

            // Inferior Derecha
            vec3 c2 = texture(screenTexture, uv + vec2(-i, -j) * src_size).rgb;
            m[2] += c2;
            s[2] += c2 * c2;

            // Inferior Izquierda
            vec3 c3 = texture(screenTexture, uv + vec2(i, -j) * src_size).rgb;
            m[3] += c3;
            s[3] += c3 * c3;
        }
    }

    float min_sigma2 = 1e+20;
    vec3 best_color = vec3(0.0);

    for (int k = 0; k < 4; ++k) {
        m[k] /= n;
        s[k] = abs(s[k] / n - m[k] * m[k]);
        float sigma2 = s[k].r + s[k].g + s[k].b;

        if (sigma2 < min_sigma2) {
            min_sigma2 = sigma2;
            best_color = m[k];
        }
    }

    FragColor = vec4(best_color, 1.0);
}