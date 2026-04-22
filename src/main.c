#define SDL_MAIN_HANDLED

#include "core/engine.h"
#include "core/gestor_memoria.h"
#include "graphics/layout.h"
#include "graphics/escenas/escena1.h"
#include "graphics/loader_obj.h"
#include "graphics/camara.h"

#define VENTANA_ANCHO 800
#define VENTANA_ALTO 600
#define TITULO_VENTANA "Motor 3D"

int main(int argc, char *argv[])
{

    Engine motor;
    if (!engine_init(&motor, TITULO_VENTANA, VENTANA_ANCHO, VENTANA_ALTO))
    {
        return 1;
    }

    Camara camara;
    inicializar_camara(&camara);

    GestorMemoria gestor_memoria;
    init_memoria(&gestor_memoria);

    Modelo **modelos_globales = inicializar_catalogo_modelos(&gestor_memoria.arena_objects);

    Layout layout;
    calcular_layout(&layout, VENTANA_ANCHO, VENTANA_ALTO, camara.zoom);

    Escena *escena_actual = cargar_escena_1(&gestor_memoria.arena_escena, modelos_globales, TOTAL_MODELOS);

    while (motor.corriendo)
    {
        engine_actualizar_eventos(&motor);
        engine_actualizar_dt(&motor);

        actualizar_escena(escena_actual, motor.dt);

        SDL_SetRenderDrawColor(motor.renderer, TEMA_DEFAULT.fondo.r, TEMA_DEFAULT.fondo.g, TEMA_DEFAULT.fondo.b, 255);
        SDL_RenderClear(motor.renderer);

        pintar_escena(escena_actual, motor.renderer, camara.distancia, &layout);
        pintar_layout(motor.renderer, &layout);

        // Textos (Usando el renderer del motor)
        SDL_SetRenderDrawColor(motor.renderer, TEMA_DEFAULT.modelo.r, TEMA_DEFAULT.modelo.g, TEMA_DEFAULT.modelo.b, 255);
        SDL_RenderDebugText(motor.renderer, 20, (float)VENTANA_ALTO - (layout.barra_h / 2) - 4, "SISTEMA: LISTO | ASSET_VAL: 50.000 EUR");

        SDL_RenderPresent(motor.renderer);
    }

    engine_limpiar(&motor);
    liberar_memoria(&gestor_memoria);

    return 0;
}