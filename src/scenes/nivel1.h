#ifndef NIVEL_1_H
#define NIVEL_1_H

#define NUM_MODELOS 1

static inline void cargar_escena_nivel_1(Arena *arena, Escena *escena, Camara *cam)
{
    crear_escena(arena, NUM_MODELOS, escena);
    crear_camara_defecto(cam);

    GLuint shaderCigarro = create_custom_shader_program("src/shaders/cigarro.vert", "src/shaders/cigarro.frag");
    Modelo *m = escena_añadir_modelo(escena, arena, "assets/models/Cigarro.obj", shaderCigarro);

    if (m)
    {
        m->posicion[2] = 1.5f;
        m->rotacion[1] = -45.0f;

        
        GLuint tex_color = cargar_textura("assets/textures/mapa_textura.png");
        modelo_add_texture(m, tex_color, "u_texture");

        GLuint tex_brillo = cargar_textura("assets/textures/cigarro_mask.png");
        modelo_add_texture(m, tex_brillo, "u_mask");

        // Reservamos memoria para los datos del controlador en la Arena
        CigarroController *c_data = (CigarroController *)arena_push(arena, sizeof(CigarroController));
        cigarro_controller_init(c_data);

        // Asignamos las funciones al modelo
        m->controller = c_data;
        m->controller_update = cigarro_update;
        m->handle_event = cigarro_handle_event;

        // Setup inicial del shader
        modelo_set_vec3(m, "colorBrasa", 181.0f / 255.0f, 55.0f / 255.0f, 38.0f / 255.0f);
    }
}

#endif