#ifndef CIGARRO_CONTROLLER_H
#define CIGARRO_CONTROLLER_H

typedef struct {
    bool space_pressed;
    float intensidad_actual;
    float velocidad_transicion;
    float intensidad_min;
    float intensidad_max;
} CigarroController;

// Inicialización de datos
static inline void cigarro_controller_init(CigarroController *ctrl) {
    ctrl->space_pressed = false;
    ctrl->intensidad_actual = 0.00f;
    ctrl->velocidad_transicion = 0.30f;
    ctrl->intensidad_min = 0.10f;
    ctrl->intensidad_max = 1.5f;
}

// Implementación del Input
static inline void cigarro_handle_event(void* ctrl_ptr, SDL_Event* e) {
    CigarroController* ctrl = (CigarroController*)ctrl_ptr;
    if (e->type == SDL_EVENT_KEY_DOWN && e->key.key == SDLK_SPACE) ctrl->space_pressed = true;
    if (e->type == SDL_EVENT_KEY_UP && e->key.key == SDLK_SPACE)   ctrl->space_pressed = false;
}

// Implementación del Update
static inline void cigarro_update(void* ctrl_ptr, float dt, void* modelo_ptr) {
    CigarroController* ctrl = (CigarroController*)ctrl_ptr;
    Modelo* m = (Modelo*)modelo_ptr;

    float objetivo = ctrl->space_pressed ? ctrl->intensidad_max : ctrl->intensidad_min;
    
    // Suavizado
    if (ctrl->intensidad_actual < objetivo) {
        ctrl->intensidad_actual += ctrl->velocidad_transicion * dt;
        if (ctrl->intensidad_actual > objetivo) ctrl->intensidad_actual = objetivo;
    } else {
        ctrl->intensidad_actual -= ctrl->velocidad_transicion * dt;
        if (ctrl->intensidad_actual < objetivo) ctrl->intensidad_actual = objetivo;
    }

    modelo_set_float(m, "intensidadBrasa", ctrl->intensidad_actual);
}

#endif