#ifndef LAYOUT_H
#define LAYOUT_H

#include <SDL3/SDL.h>

typedef struct {
    int menu_w;      // Ancho del panel izquierdo
    int barra_h;     // Alto de la barra de estado inferior
    int area_util_h; // Altura disponible para el menú
    int juego_w;     // Ancho real del lienzo 3D
    int juego_h;     // Alto real del lienzo 3D
    SDL_Rect viewport_juego; // El rectángulo ya listo para SDL

    float escala_proyeccion; // El factor base según la resolución
    float zoom_usuario;      // Para que puedas alejar/acercar la cámara
    SDL_FPoint centro_juego; // El punto central del viewport (x, y)
} Layout;

void calcular_layout(Layout *l, int win_w, int win_h, float zoom);
void pintar_layout(SDL_Renderer *renderer, Layout *l);

#endif