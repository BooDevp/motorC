#ifndef APP_H
#define APP_H

#include "core/engine.h"
#include "core/gestor_memoria.h"
#include "graphics/layout.h"
#include "graphics/camara.h"
#include "graphics/escena.h"

typedef struct
{
    Engine motor;
    GestorMemoria memoria;
    Camara camara;
    Layout layout;
    Escena *escena_actual;
    Modelo **catalogo;
} App;

bool app_init(App *app);
void app_run(App *app);
void app_shutdown(App *app);

#endif