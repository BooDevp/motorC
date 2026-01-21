#ifndef UI_H
#define UI_H

#include <SDL3/SDL.h>

typedef struct
{
    float x, y, w, h;
    SDL_Color color;    
    const char *etiqueta;
    void (*accion)(void *params);
    void *params;
} Boton;

void ui_dibujar_boton(SDL_Renderer *renderer, Boton *b, void *params, bool *hover_any_btn);
void ui_init();
void gestionar_cursor_raton(bool hover_any_btn);

#endif