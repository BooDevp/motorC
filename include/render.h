#ifndef RENDER_H
#define RENDER_H

/**
 * Renderiza un frame
 */
static inline void render_frame(GraphicsState *gs, Camara *cam, Modelo *modelo, AppState *app, int width, int height)
{    

    // 2. Configurar el modo de dibujo (Sólido o Wireframe)
    glPolygonMode(GL_FRONT_AND_BACK, app->wireframe ? GL_LINE : GL_FILL);

    // 3. ACTUALIZAR MATRICES (Ahora dentro)
    // Esto calcula el MVP y lo sube a la GPU para el modelo que vamos a dibujar ahora
    setup_matrices(gs, width, height, modelo, cam);

    // 4. Dibujar el modelo
    glUseProgram(gs->program);
    glBindVertexArray(modelo->vao);
    glDrawArrays(GL_TRIANGLES, 0, modelo->num_vertices);

    // 5. Limpiar estado (Opcional pero recomendado)
    glBindVertexArray(0);
    glUseProgram(0);
}


#endif