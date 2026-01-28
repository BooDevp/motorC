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
    GLuint shaderCigarro = create_custom_shader_program("src/shaders/simple.vert", "src/shaders/cigarro.frag");

    Modelo *h1 = escena_añadir_modelo(&escena, arena, "assets/models/Cigarro.obj", shaderCigarro);
    if (h1)
    {
        h1->posicion[2] = 1.5f;
        // Esta es la textura que pintaste en Blender donde la punta es blanca y el resto negro
        h1->textura_id = cargar_textura("assets/textures/cigarro_mask.png");
        h1->rotacion[1] = -90.0f;
        
        // Configurar parámetros dinámicos del shader
        modelo_set_float(h1, "intensidadBrasa", 0.5f);
        modelo_set_float(h1, "velocidadLatido", 0.25f);
        modelo_set_vec3(h1, "colorBrasa", 1.0f, 0.25f, 0.0f);
    }   

    debug_log("Nivel 1 cargado: %d modelos", escena.cantidad);

    return escena;
}

#endif