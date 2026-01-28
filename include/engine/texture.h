#ifndef TEXTURE_H
#define TEXTURE_H

#define STB_IMAGE_IMPLEMENTATION
#include "external/stb_image.h"

static inline GLuint cargar_textura(const char *ruta) {
    int width, height, nrChannels;
    // Forzamos 4 canales para evitar problemas de alineación
    unsigned char *data = stbi_load(ruta, &width, &height, &nrChannels, 4); 
    
    if (!data) {
        debug_log("Error al cargar textura: %s", ruta);
        return 0;
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // CONFIGURACIÓN PARA MÁSCARAS (Evita huecos y repeticiones)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    // Usamos LINEAR para que el borde no sea un escalón de sierra, 
    // pero sin MIPMAPS para que no se emborrone a lo lejos.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    stbi_image_free(data);
    return textureID;
}

#endif