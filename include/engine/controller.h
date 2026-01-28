#ifndef CONTROLLER_H
#define CONTROLLER_H

// Estructura para envolver la lógica genérica
typedef struct
{
    void *data;
    void (*update)(void *ctrl, float dt, void *m);
    void (*handle_event)(void *ctrl, SDL_Event *e);
} ControllerInterface;

#endif