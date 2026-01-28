/**
 * controller.h
 * Sistema de control de input para interacción con modelos y shaders
 */

#ifndef CIGARRO_CONTROLLER_H
#define CIGARRO_CONTROLLER_H

#include <stdbool.h>

// Estado del controlador del cigarro
typedef struct
{
    bool space_pressed;         // Estado actual de la tecla espacio
    float intensidad_actual;    // Intensidad actual (0.0 - 1.0)
    float intensidad_objetivo;  // Intensidad objetivo (0.0 - 1.0)
    float velocidad_transicion; // Velocidad de transición (unidades por segundo)

    // Valores del shader
    float intensidad_min; // Intensidad mínima (apagado)
    float intensidad_max; // Intensidad máxima (encendido)
} CigarroController;

/**
 * Inicializa el controlador del cigarro
 */
static inline void cigarro_controller_init(CigarroController *cigarro_ctrl)
{
    cigarro_ctrl->space_pressed = false;
    cigarro_ctrl->intensidad_actual = 0.5f; // Empieza apagado (brillo muy bajo)
    cigarro_ctrl->intensidad_objetivo = 0.5f;
    cigarro_ctrl->velocidad_transicion = 3.0f; // Transición en ~0.5 segundos

    // Valores del shader
    cigarro_ctrl->intensidad_min = 0.5f; // Brillo mínimo (cigarro apagado)
    cigarro_ctrl->intensidad_max = 4.0f; // Brillo máximo (cigarro encendido)
}

/**
 * Inicializa los parámetros del shader del modelo
 * Llamar UNA VEZ después de crear el modelo
 */
static inline void cigarro_controller_init_shader(CigarroController *ctrl, Modelo *cigarro)
{
    // Configurar parámetros iniciales del shader
    modelo_set_float(cigarro, "intensidadBrasa", ctrl->intensidad_min);
    modelo_set_float(cigarro, "velocidadLatido", 3.0f); // Velocidad base
    modelo_set_vec3(cigarro, "colorBrasa", 1.0f, 0.25f, 0.0f);
}

/**
 * Función auxiliar de interpolación lineal (lerp)
 */
static inline float lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}

/**
 * Actualiza el estado del controlador (llamar cada frame)
 */
static inline void cigarro_controller_update(CigarroController *ctrl, float delta_time)
{
    // Determinar intensidad objetivo según el input
    if (ctrl->space_pressed)
    {
        ctrl->intensidad_objetivo = ctrl->intensidad_max;
    }
    else
    {
        ctrl->intensidad_objetivo = ctrl->intensidad_min;
    }

    // Interpolar suavemente hacia el objetivo
    float diferencia = ctrl->intensidad_objetivo - ctrl->intensidad_actual;
    float cambio = ctrl->velocidad_transicion * delta_time;

    if (diferencia > 0)
    {
        // Encendiendo
        ctrl->intensidad_actual += cambio;
        if (ctrl->intensidad_actual > ctrl->intensidad_objetivo)
            ctrl->intensidad_actual = ctrl->intensidad_objetivo;
    }
    else if (diferencia < 0)
    {
        // Apagando
        ctrl->intensidad_actual -= cambio;
        if (ctrl->intensidad_actual < ctrl->intensidad_objetivo)
            ctrl->intensidad_actual = ctrl->intensidad_objetivo;
    }
}

/**
 * Aplica el estado del controlador a un modelo con shader de cigarro
 */
static inline void cigarro_controller_apply(CigarroController *ctrl, Modelo *cigarro)
{
    // Actualizar parámetro de intensidad del shader
    modelo_set_float(cigarro, "intensidadBrasa", ctrl->intensidad_actual);

    // Opcional: También podemos modificar la velocidad del latido
    // Cuando está más encendido, late más rápido
    float velocidad_latido = lerp(3.0f, 6.0f,
                                  (ctrl->intensidad_actual - ctrl->intensidad_min) /
                                      (ctrl->intensidad_max - ctrl->intensidad_min));
    modelo_set_float(cigarro, "velocidadLatido", velocidad_latido);
}

/**
 * Maneja el evento de tecla presionada
 */
static inline void cigarro_controller_key_down(CigarroController *ctrl, int key)
{
    if (key == SDLK_SPACE)
    {
        ctrl->space_pressed = true;
    }
}

/**
 * Maneja el evento de tecla liberada
 */
static inline void cigarro_controller_key_up(CigarroController *ctrl, int key)
{
    if (key == SDLK_SPACE)
    {
        ctrl->space_pressed = false;
    }
}

/**
 * Función wrapper para usar como callback en Modelo
 * Actualiza el controller Y aplica los cambios al modelo
 */
static inline void cigarro_controller_update_and_apply(void *ctrl_ptr, float delta_time, void *modelo_ptr)
{
    CigarroController *ctrl = (CigarroController *)ctrl_ptr;
    Modelo *modelo = (Modelo *)modelo_ptr;

    cigarro_controller_update(ctrl, delta_time);
    cigarro_controller_apply(ctrl, modelo);
}

#endif // CONTROLLER_H
