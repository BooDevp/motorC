#ifndef APPSTATE_H
#define APPSTATE_H

typedef struct
{
    bool wireframe;
    bool running;
    bool postprocesado;
} AppState;

static inline AppState init_appstate(){
    AppState app = {
        .wireframe = false,
        .running = true,
        .postprocesado = true,
    };
    return app;
} 

#endif