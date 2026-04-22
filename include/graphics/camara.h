#ifndef CAMARA_H
#define CAMARA_H

typedef struct {
    float distancia;
    float zoom;
} Camara;

void inicializar_camara(Camara *camara);

#endif