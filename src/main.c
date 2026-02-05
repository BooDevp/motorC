#define SOKOL_IMPL
#include "engine/engine.h"

#define ARENA_SIZE_MB 10
#define ANCHO_PANTALLA 600
#define ALTO_PANTALLA 800
#define TITULO_VENTANA "PS1 Engine - Sokol"

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    Engine engine = {0};

    // Arrancamos el motor
    if (!engine_init(&engine, TITULO_VENTANA, ALTO_PANTALLA, ANCHO_PANTALLA, ARENA_SIZE_MB))
    {
        return 1;
    }

    // Bucle principal
    while (engine.running)
    {

        engine_update(&engine);

        // Renderizado Sokol
        sg_begin_pass(&(sg_pass){
            .action = engine.pass_action,
            .swapchain = {.width = engine.width, .height = engine.height}});

        sg_end_pass();
        sg_commit();

        SDL_GL_SwapWindow(engine.window);
    }

    engine_cleanup(&engine);
    return 0;
}