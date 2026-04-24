#ifndef GESTOR_EVENTOS_H
#define GESTOR_EVENTOS_H

#include <SDL3/SDL.h>
#include "core/engine.h"
#include "ui/menu_lateral.h"

void app_procesar_eventos(Engine *motor, MenuLateral *menu);

#endif