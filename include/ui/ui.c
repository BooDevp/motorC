#include <stdio.h>
#include "ui.h"

// Saber si el ratón está encima del botón
static bool is_mouse_hover(float mx, float my, Boton b)
{
    return (mx >= b.x && mx <= b.x + b.w &&
            my >= b.y && my <= b.y + b.h);
}

// Dibujar un botón en pantalla
void ui_dibujar_boton(SDL_Renderer *renderer, Boton *b, void *params, bool *hover_any_btn)
{

    float mx, my;
    uint32_t estado_raton = SDL_GetMouseState(&mx, &my);
    bool hover = is_mouse_hover(mx, my, *b);

    if (hover)
    {
        b->color = (SDL_Color){0, 200, 0, 255};
        if (estado_raton & SDL_BUTTON_LMASK)
        {
            // Si el botón tiene una función asignada, la ejecutamos
            if (b->accion != NULL)
            {
                b->accion(params);
            }
        }
    }
    else
    {
        b->color = (SDL_Color){100, 100, 100, 255};
    }

    // 1. Color del fondo
    SDL_SetRenderDrawColor(renderer, b->color.r, b->color.g, b->color.b, 255);

    // 2. Rectángulo
    SDL_FRect rect_btn = {b->x, b->y, b->w, b->h};
    SDL_RenderFillRect(renderer, &rect_btn);

    // 3. Texto (siempre blanco por ahora)
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDebugText(renderer, b->x + 10, b->y + 12, b->etiqueta);

    if (hover)
    {
        *hover_any_btn = true;
    }
}
