#include "graphics/layout.h"

#define RATIO_MENU_LATERAL 0.25f
#define RATIO_BARRA_INFERIOR 0.15f

void calcular_layout(Layout *l, int win_w, int win_h, float zoom)
{
    l->menu_w = (int)(win_w * RATIO_MENU_LATERAL);
    l->barra_h = (int)(win_h * RATIO_BARRA_INFERIOR);
    l->area_util_h = win_h - l->barra_h;
    l->juego_w = win_w - l->menu_w;
    l->juego_h = l->area_util_h;

    l->viewport_juego.x = l->menu_w;
    l->viewport_juego.y = 0;
    l->viewport_juego.w = l->juego_w;
    l->viewport_juego.h = l->juego_h;

    l->zoom = zoom;
    l->escala_proyeccion = (l->juego_w / 2.0f) * l->zoom;

    l->centro_juego.x = l->juego_w / 2.0f;
    l->centro_juego.y = l->juego_h / 2.0f;
}

void pintar_layout(SDL_Renderer *renderer, Layout *layout, int ventana_alto)
{
    SDL_SetRenderDrawColor(renderer, TEMA_DEFAULT.modelo.r, TEMA_DEFAULT.modelo.g, TEMA_DEFAULT.modelo.b, 255);

    // Línea vertical (Separa menú de juego)
    SDL_RenderLine(renderer, (float)layout->menu_w, 0, (float)layout->menu_w, (float)layout->area_util_h);

    // Línea horizontal (Separa juego de barra inferior)
    float ancho_total = (float)(layout->menu_w + layout->juego_w);
    SDL_RenderLine(renderer, 0, (float)layout->area_util_h, ancho_total, (float)layout->area_util_h);
    
    SDL_SetRenderDrawColor(renderer, TEMA_DEFAULT.modelo.r, TEMA_DEFAULT.modelo.g, TEMA_DEFAULT.modelo.b, 255);
    SDL_RenderDebugText(renderer, 20, (float)ventana_alto - (layout->barra_h / 2) - 4, "SISTEMA: LISTO | ASSET_VAL: 50.000 EUR");
}