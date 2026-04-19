#ifndef MODELOS_ID_H
#define MODELOS_ID_H

typedef enum {
    MODELO_HANDPOPUS,
    MODELO_CUBO,
    TOTAL_MODELOS
} ModeloID;

static const char *rutas_modelos_globales[] = {
    [MODELO_HANDPOPUS] = "./assets/models/Handpopus.obj",
    [MODELO_CUBO]      = "./assets/models/Cubo.obj",    
};

#endif