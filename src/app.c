#include "app.h"
#include "graphics/escenas/escena1.h"
#include "graphics/loader_obj.h"
#include "core/gestor_eventos.h"

#define VENTANA_ANCHO 800
#define VENTANA_ALTO 600
#define TITULO_VENTANA "Motor 3D"

bool app_init(App *app)
{
    if (!engine_init(&app->motor, TITULO_VENTANA, VENTANA_ANCHO, VENTANA_ALTO))
    {
        return false;
    }

    inicializar_camara(&app->camara);
    init_memoria(&app->memoria);

    app->catalogo = inicializar_catalogo_modelos(&app->memoria.arena_objects);
    calcular_layout(&app->layout, VENTANA_ANCHO, VENTANA_ALTO, app->camara.zoom);

    app->escena_actual = cargar_escena_1(&app->memoria.arena_escena, app->catalogo, TOTAL_MODELOS);

    menu_lateral_init(&app->menu, &app->layout);

    app->cursor_flecha = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);
    app->cursor_mano = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_POINTER);

    return true;
}

void app_run(App *app)
{
    while (app->motor.corriendo)
    {
        // GESTIÓN DE EVENTOS
        app_procesar_eventos(&app->motor, &app->menu);

        // LÓGICA DE CURSOR (Centralizada)
        bool sobre_ui = false;

        // Comprobamos si algún botón del menú tiene el ratón encima
        for (int i = 0; i < app->menu.contador_botones; i++)
        {
            if (app->menu.botones[i].is_hovered)
            {
                sobre_ui = true;
                break;
            }
        }

        // Si estamos sobre UI o sobre un modelo (en el futuro), cambiamos el cursor
        if (sobre_ui)
        {
            SDL_SetCursor(app->cursor_mano);
        }
        else
        {
            SDL_SetCursor(app->cursor_flecha);
        }

        // Obtener posición del ratón para interacciones UI
        float mx, my;
        SDL_GetMouseState(&mx, &my);
        menu_lateral_actualizar(&app->menu, mx, my);

        // Tiempo
        engine_actualizar_dt(&app->motor);

        // Lógica de la escena
        actualizar_escena(app->escena_actual, app->motor.dt);

        // Renderizar escena y layout
        pintar_escena(app->escena_actual, app->motor.renderer, app->camara.distancia, &app->layout);
        pintar_layout(app->motor.renderer, &app->layout, VENTANA_ALTO);

        // Menú lateral
        menu_lateral_dibujar(app->motor.renderer, &app->menu);

        // Presentar todo
        SDL_RenderPresent(app->motor.renderer);
    }
}

void app_shutdown(App *app)
{
    SDL_DestroyCursor(app->cursor_flecha);
    SDL_DestroyCursor(app->cursor_mano);
    
    engine_limpiar(&app->motor);
    liberar_memoria(&app->memoria);
}