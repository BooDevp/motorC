#include "graphics/camara.h"

#define DISTANCIA_CAMARA 1.0f
#define ZOOM 1.0f

void inicializar_camara(Camara *camara)
{
    camara->distancia = DISTANCIA_CAMARA;
    camara->zoom = ZOOM;
}