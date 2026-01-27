#ifndef RENDER_H
#define RENDER_H

/**
 * Renderiza un frame
 */
static inline void render_frame(GraphicsState *gs, Camara *cam, Modelo *modelo, AppState *app, int width, int height)
{

    // Configurar el modo de dibujo (Sólido o Wireframe)
    glPolygonMode(GL_FRONT_AND_BACK, app->wireframe ? GL_LINE : GL_FILL);

    // Esto calcula el MVP y lo sube a la GPU para el modelo que vamos a dibujar ahora
    setup_matrices(gs, width, height, modelo, cam);

    // Dibujar el modelo
    glBindVertexArray(modelo->vao);
    glDrawArrays(GL_TRIANGLES, 0, modelo->num_vertices);
}

#endif