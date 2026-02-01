#define SOKOL_IMPL // Solo se define la implementación aquí
#include "engine/engine.h"

int main(int argc, char* argv[]) {
    (void)argc; (void)argv;

    Engine engine = {0};

    // Arrancamos el motor
    if (!engine_init(&engine, "PS1 Engine - Sokol", 800, 600)) {
        return 1;
    }

    // Bucle principal
    while (engine.running) {
        
        engine_update(&engine);

        // Renderizado Sokol
        sg_begin_pass(&(sg_pass){ 
            .action = engine.pass_action, 
            .swapchain = { .width = engine.width, .height = engine.height } 
        });
        
        // --- Aquí irá tu código de dibujo de triángulos ---
        
        sg_end_pass();
        sg_commit();

        SDL_GL_SwapWindow(engine.window);
    }

    engine_cleanup(&engine);
    return 0;
}