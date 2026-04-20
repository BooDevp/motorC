#ifndef MODELOS_ID_H
#define MODELOS_ID_H

typedef enum {
    MODELO_HANDPOPUS,
    MODELO_CUBO,
    TOTAL_MODELOS
} ModeloID;

extern const char *rutas_modelos_globales[TOTAL_MODELOS];

#endif