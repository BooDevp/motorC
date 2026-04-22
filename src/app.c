#include "app.h"
#include "graphics/escenas/escena1.h"
#include "graphics/loader_obj.h"

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

    return true;
}

void app_run(App *app)
{
    while (app->motor.corriendo)
    {
        // Eventos y tiempo
        engine_actualizar_eventos(&app->motor);
        engine_actualizar_dt(&app->motor);

        // Lógica de la escena
        actualizar_escena(app->escena_actual, app->motor.dt);

        // Renderizar escena y layout        
        pintar_escena(app->escena_actual, app->motor.renderer, app->camara.distancia, &app->layout);
        pintar_layout(app->motor.renderer, &app->layout);

        // Barra de estado
        SDL_SetRenderDrawColor(app->motor.renderer, TEMA_DEFAULT.modelo.r, TEMA_DEFAULT.modelo.g, TEMA_DEFAULT.modelo.b, 255);
        SDL_RenderDebugText(app->motor.renderer, 20, (float)VENTANA_ALTO - (app->layout.barra_h / 2) - 4, "SISTEMA: LISTO | ASSET_VAL: 50.000 EUR");

        // Presentar todo
        SDL_RenderPresent(app->motor.renderer);
    }
}

void app_shutdown(App *app)
{
    engine_limpiar(&app->motor);
    liberar_memoria(&app->memoria);
}