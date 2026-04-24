#include <stdio.h>
#include "ui/menu_lateral.h"

// Funciones de acción
void accion_reset()
{
    printf("SISTEMA: Posicion de camara reiniciada.\n");
}

void accion_save()
{
    printf("SISTEMA: Datos guardados en disco.\n");
}

void menu_lateral_init(MenuLateral *menu, Layout *layout)
{
    menu->contador_botones = 0;

    float btn_w = (float)layout->menu_w - 20;
    float btn_x = 10;

    // Crear botones uno debajo de otro
    boton_crear(&menu->botones[0], btn_x, 20, btn_w, 40, "RESET VIEW", accion_reset);
    boton_crear(&menu->botones[1], btn_x, 70, btn_w, 40, "SAVE STATE", accion_save);
    boton_crear(&menu->botones[2], btn_x, 120, btn_w, 40, "LOAD STATE", NULL);

    menu->contador_botones = 3;
}

void menu_lateral_gestionar_clic(MenuLateral *menu, float mouse_x, float mouse_y)
{
    for (int i = 0; i < menu->contador_botones; i++)
    {
        // Esta es la función que definimos antes en boton.c
        boton_intentar_clic(&menu->botones[i], mouse_x, mouse_y);
    }
}

void menu_lateral_actualizar(MenuLateral *menu, float mouse_x, float mouse_y)
{
    for (int i = 0; i < menu->contador_botones; i++)
    {
        boton_actualizar(&menu->botones[i], mouse_x, mouse_y);
    }
}

void menu_lateral_dibujar(SDL_Renderer *renderer, MenuLateral *menu)
{
    for (int i = 0; i < menu->contador_botones; i++)
    {
        boton_dibujar(renderer, &menu->botones[i]);
    }
}