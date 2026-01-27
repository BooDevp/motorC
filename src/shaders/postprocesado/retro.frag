#version 330 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D screenTexture;
uniform float iTime;
uniform vec2 iResolution;

// Configuración optimizada
const float wiggle = 0.03;
const float wiggle_speed = 25.0;
const float smear = 1.0;
const int blur_samples = 5; // Suficiente para un look retro fluido

// Funciones auxiliares (se mantienen igual)
float onOff(float a, float b, float c, float framecount) {
    return step(c, sin((framecount * 0.001) + a * cos((framecount * 0.001) * b)));
}

vec2 jumpy(vec2 uv, float framecount) {
    vec2 look = uv;
    float window = 1.0 / (1.0 + 80.0 * (look.y - mod(framecount / 4.0, 1.0)) * (look.y - mod(framecount / 4.0, 1.0)));
    look.x += 0.05 * sin(look.y * 10.0 + framecount) / 20.0 * onOff(4.0, 4.0, 0.3, framecount) * (0.5 + cos(framecount * 20.0)) * window;
    float vShift = (0.1 * wiggle) * 0.4 * onOff(2.0, 3.0, 0.9, framecount) * (sin(framecount) * sin(framecount * 20.0) + (0.5 + 0.1 * sin(framecount * 200.0) * cos(framecount)));
    look.y = mod(look.y - 0.01 * vShift, 1.0);
    return look;
}

vec2 Circle(float Start, float Points, float Point) {
    float Rad = (3.141592 * 2.0 * (1.0 / Points)) * (Point + Start);
    return vec2(-(.3 + Rad), cos(Rad));
}

vec3 rgb2yiq(vec3 c) {
    return vec3(
        (0.2989 * c.x + 0.5959 * c.y + 0.2115 * c.z),
        (0.5870 * c.x - 0.2744 * c.y - 0.5229 * c.z),
        (0.1140 * c.x - 0.3216 * c.y + 0.3114 * c.z)
    );
}

vec3 yiq2rgb(vec3 c) {
    return vec3(
        (1.0 * c.x + 1.0 * c.y + 1.0 * c.z),
        (0.956 * c.x - 0.2720 * c.y - 1.1060 * c.z),
        (0.6210 * c.x - 0.6474 * c.y + 1.7046 * c.z)
    );
}

void main() {
    vec2 uv = TexCoords;

    // Lógica de distorsión temporal
    float d = 0.1 - round(mod(iTime / 3.0, 1.0)) * 0.1;
    uv = jumpy(uv, mod(iTime * wiggle_speed, 7.0));

    float s = 0.0001 * -d + 0.0001 * wiggle * (sin(iTime * wiggle_speed));
    float e = min(.30, pow(max(0.0, cos(uv.y * 4.0 + .3) - .75) * (s + 0.5) * 1.0, 3.0)) * 25.0;
    float r = (250.0 * (2.0 * s));
    uv.x += abs(r * pow(min(.003, (-uv.y + (.01 * mod(iTime, 5.0)))) * 3.0, 2.0)) * wiggle;
    
    d = 0.051 + abs(sin(s / 4.0));
    float c_base = max(0.0001, .002 * d) * smear;
    
    // --- BUCLE OPTIMIZADO: Una sola pasada para Y, I y Q ---
    float y = 0.0, i_comp = 0.0, q_comp = 0.0;
    float W = 1.0 / float(blur_samples);
    float Start = 2.0 / float(blur_samples);

    for (int n = 0; n < blur_samples; ++n) {
        vec2 offset_dir = Circle(Start, float(blur_samples), float(n));
        
        // Muestra para Y
        vec2 uvY = uv + offset_dir * (0.66 * 4.0 * 2.0 * vec2(c_base + c_base * uv.x, 0.0));
        y += rgb2yiq(texture(screenTexture, uvY).rgb).r * W;

        // Muestra para I
        vec2 uvI = (uv + vec2(0.01 * d, 0.0)) + offset_dir * (0.66 * 4.0 * 2.0 * vec2(c_base * 6.0, 0.0));
        i_comp += rgb2yiq(texture(screenTexture, uvI).rgb).g * W;

        // Muestra para Q
        vec2 uvQ = (uv + vec2(0.015 * d, 0.0)) + offset_dir * (0.66 * 4.0 * 2.0 * vec2(c_base * 15.0, 0.0));
        q_comp += rgb2yiq(texture(screenTexture, uvQ).rgb).b * W;
    }

    vec3 finalColor = yiq2rgb(vec3(y, i_comp, q_comp)) - pow(s + e * 2.0, 3.0);
    FragColor = vec4(finalColor, 1.0);
}