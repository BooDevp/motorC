#ifndef NIVEL_1_H
#define NIVEL_1_H

#include "engine/escena.h"
#include "engine/shader.h"

#define NUM_MODELOS 2

// Escena 1
static inline void cargar_escena_nivel_1(Arena *arena, Escena *escena, Camara *cam, CigarroController *cigarro_ctrl)
{
    // INIT
    crear_escena(arena, NUM_MODELOS, escena);
    crear_camara_defecto(cam);
    cigarro_controller_init(cigarro_ctrl);

    // Shader custom
    GLuint shaderCigarro = create_custom_shader_program("src/shaders/simple.vert", "src/shaders/cigarro.frag");

    Modelo *h1 = escena_añadir_modelo(escena, arena, "assets/models/Cigarro.obj", shaderCigarro);
    if (h1)
    {
        h1->posicion[2] = 1.5f;
        h1->rotacion[1] = -90.0f;
        h1->controller = cigarro_ctrl;
        h1->controller_update = cigarro_controller_update_and_apply;

        // Esta es la textura que pintaste en Blender donde la punta es blanca y el resto negro
        h1->textura_id = cargar_textura("assets/textures/cigarro_mask.png");

        cigarro_controller_init_shader(cigarro_ctrl, h1);
    }

    debug_log("Nivel 1 cargado: %d modelos", escena->cantidad);
}

#endif