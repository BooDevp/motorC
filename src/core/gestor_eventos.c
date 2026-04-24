#include "core/gestor_eventos.h"

void app_procesar_eventos(Engine *motor, MenuLateral *menu)
{
    SDL_Event ev;
    while (SDL_PollEvent(&ev))
    {

        if (ev.type == SDL_EVENT_QUIT)
        {
            motor->corriendo = false;
        }

        // Teclado
        if (ev.type == SDL_EVENT_KEY_DOWN && ev.key.key == SDLK_ESCAPE)
        {
            motor->corriendo = false;
        }

        // Ratón
        if (ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
        {
            float mx, my;
            SDL_GetMouseState(&mx, &my);

            if (ev.button.button == SDL_BUTTON_LEFT)
            {
                menu_lateral_gestionar_clic(menu, mx, my);
            }
        }
    }
}