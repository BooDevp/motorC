#ifndef NIVEL_1_H
#define NIVEL_1_H

#include "engine/escena.h"
#include "engine/shader.h"

#define NUM_MODELOS 2

// Escena 1
static inline Escena cargar_escena_nivel_1(Arena *arena)
{
    // Creamos la escena con capacidad
    Escena escena = crear_escena(arena, NUM_MODELOS);

    // Shader custom
    GLuint shaderRojo = create_custom_shader_program("src/shaders/simple.vert", "src/shaders/cigarro.frag");

    Modelo *h1 = escena_añadir_modelo(&escena, arena, "assets/models/Cigarro.obj", shaderRojo);
    if (h1)
    {
        h1->posicion[2] = 1.5f;
        // Esta es la textura que pintaste en Blender donde la punta es blanca y el resto negro
        h1->textura_id = cargar_textura("assets/textures/cigarro_mask.png");
    }   

    debug_log("Nivel 1 cargado: %d modelos", escena.cantidad);

    return escena;
}

#endif