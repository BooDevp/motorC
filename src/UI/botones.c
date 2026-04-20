#define SDL_MAIN_HANDLED
#include "ui/botones.h"

bool g_hover_any_btn = false;
SDL_Cursor *cursor_mano = NULL;
SDL_Cursor *cursor_normal = NULL;

bool is_mouse_hover(float mx, float my, Boton b)
{
    return (mx >= b.x && mx <= b.x + b.w &&
            my >= b.y && my <= b.y + b.h);
}

void ui_comenzar_frame()
{
    g_hover_any_btn = false;
}

void ui_dibujar_botones_menu_lateral(SDL_Renderer *renderer, Boton *b, void *params)
{
    float mx, my;
    uint32_t estado_raton = SDL_GetMouseState(&mx, &my);
    bool hover = is_mouse_hover(mx, my, *b);

    // Colores TEMA_DEFAULT
    SDL_Color color_eje = hover ? TEMA_DEFAULT.modelo : TEMA_DEFAULT.marco;
    Uint8 alpha_fondo = hover ? 40 : 15; // Brillo sutil al pasar el raton

    if (hover)
    {
        g_hover_any_btn = true;
        if (estado_raton & SDL_BUTTON_LMASK)
        {
            if (b->accion != NULL) b->accion(params);
        }
    }

    SDL_FRect rect = {b->x, b->y, b->w, b->h};

    // FONDO SEMI-TRANSPARENTE
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, color_eje.r, color_eje.g, color_eje.b, alpha_fondo);
    SDL_RenderFillRect(renderer, &rect);

    // Esquinas reforzadas
    SDL_SetRenderDrawColor(renderer, color_eje.r, color_eje.g, color_eje.b, 255);
    
    // Dibujamos las 4 muescas de las esquinas (L-shapes)
    float len = 8.0f; // Longitud de la muesca
    // Arriba-Izquierda
    SDL_RenderLine(renderer, rect.x, rect.y, rect.x + len, rect.y);
    SDL_RenderLine(renderer, rect.x, rect.y, rect.x, rect.y + len);
    // Arriba-Derecha
    SDL_RenderLine(renderer, rect.x + rect.w, rect.y, rect.x + rect.w - len, rect.y);
    SDL_RenderLine(renderer, rect.x + rect.w, rect.y, rect.x + rect.w, rect.y + len);
    // Abajo-Izquierda
    SDL_RenderLine(renderer, rect.x, rect.y + rect.h, rect.x + len, rect.y + rect.h);
    SDL_RenderLine(renderer, rect.x, rect.y + rect.h, rect.x, rect.y + rect.h - len);
    // Abajo-Derecha
    SDL_RenderLine(renderer, rect.x + rect.w, rect.y + rect.h, rect.x + rect.w - len, rect.y + rect.h);
    SDL_RenderLine(renderer, rect.x + rect.w, rect.y + rect.h, rect.x + rect.w, rect.y + rect.h - len);

    // INDICADOR LATERAL HOVER
    if (hover) {
        SDL_FRect indicador = {rect.x + 2, rect.y + 2, 3, rect.h - 4};
        SDL_RenderFillRect(renderer, &indicador);
    }

    // TEXTO
    SDL_SetRenderDrawColor(renderer, TEMA_DEFAULT.modelo.r, TEMA_DEFAULT.modelo.g, TEMA_DEFAULT.modelo.b, 255);
    SDL_RenderDebugText(renderer, b->x + 15, b->y + (b->h / 2) - 4, b->etiqueta);
}

void gestionar_cursor_raton()
{
    if (!cursor_mano || !cursor_normal)
        return;

    if (g_hover_any_btn)
    {
        SDL_SetCursor(cursor_mano);
    }
    else
    {
        SDL_SetCursor(cursor_normal);
    }
}

void ui_init()
{
    if (!cursor_mano)
        cursor_mano = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_POINTER);
    if (!cursor_normal)
        cursor_normal = SDL_GetDefaultCursor();
}

void accion_cambiar_vsync(void *datos)
{
    FunctionCambioVsync *d = (FunctionCambioVsync *)datos;
    cambiar_vsync(d->renderer, d->bool_vsync, d->ultimo_clic, d->COOLDOWN_BOTON);
}

void accion_cargar_escena(void *datos)
{
    FunctionCargarEscena *d = (FunctionCargarEscena *)datos;
    Uint64 tiempo_actual = SDL_GetTicks();

    if (tiempo_actual - *d->ultimo_clic > d->COOLDOWN_BOTON)
    {
        switch (d->escena_id)
        {
        case 1:
            *d->escena = cargar_escena_1(d->arena, d->modelos_globales, d->n_modelos_globales);
            break;
        }
        *d->ultimo_clic = tiempo_actual;
    }
}
