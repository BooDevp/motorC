#ifndef APPSTATE_H
#define APPSTATE_H

typedef struct
{
    bool vsync;
    bool wireframe;
    bool running;
    bool postprocesado;
} AppState;

static inline AppState init_appstate(){
    AppState app = {
        .vsync = true,
        .wireframe = false,
        .running = true,
        .postprocesado = false,
    };
    return app;
} 

#endif