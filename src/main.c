#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

// --- AJUSTES ---
#define WINDOW_W 800
#define WINDOW_H 600
#define FOV_GRADOS 60.0f
#define CAM_DIST   4.0f
#define ROT_VEL    1.0f

// --- ESTRUCTURAS ---
typedef struct { float m[16]; } Mat4;

// --- DEBUG ---
static void CheckSDLError(const char* context) {
    const char* error = SDL_GetError();
    if (error && *error) {
        fprintf(stderr, "%s: %s\n", context, error);
        SDL_ClearError();
    }
}

// --- MATEMÁTICAS ---
void MatIdentidad(Mat4* m) {
    memset(m->m, 0, sizeof(float) * 16);
    m->m[0] = 1.0f; m->m[5] = 1.0f; m->m[10] = 1.0f; m->m[15] = 1.0f;
}

void MultiplicarMat(Mat4* out, const Mat4* a, const Mat4* b) {
    Mat4 r;
    for (int col = 0; col < 4; col++) {
        for (int row = 0; row < 4; row++) {
            r.m[col * 4 + row] =
                a->m[0 * 4 + row] * b->m[col * 4 + 0] +
                a->m[1 * 4 + row] * b->m[col * 4 + 1] +
                a->m[2 * 4 + row] * b->m[col * 4 + 2] +
                a->m[3 * 4 + row] * b->m[col * 4 + 3];
        }
    }
    *out = r;
}

// Matriz de proyección perspectiva con Z en [0,1] (Vulkan)
void MatrizProyeccion(float fov_grados, float aspecto, float near, float far, Mat4* out) {
    float fov_rad = fov_grados * 0.5f * (3.14159265f / 180.0f);
    float f = 1.0f / tanf(fov_rad);
    float range = 1.0f / (near - far);
    
    Mat4 proj = {0};
    proj.m[0] = f / aspecto;
    proj.m[5] = f;  // Sin flip Y para compatibilidad
    proj.m[10] = far * range;  // Para Z [0,1] en Vulkan
    proj.m[14] = far * near * range;
    proj.m[11] = -1.0f;  // Para proyección perspectiva
    
    // Corrección para mapear Z de [-1,1] a [0,1]
    Mat4 clip_correction = {0};
    clip_correction.m[0] = 1.0f;
    clip_correction.m[5] = 1.0f;
    clip_correction.m[10] = 0.5f;
    clip_correction.m[14] = 0.5f;
    clip_correction.m[15] = 1.0f;
    
    MultiplicarMat(out, &clip_correction, &proj);
}

// Matriz de vista (cámara en Z negativo mirando hacia +Z)
void MatrizVista(float distancia, Mat4* out) {
    MatIdentidad(out);
    out->m[14] = -distancia;  // Traslación en Z negativo
}

// Matriz de rotación en Y
void MatrizRotacionY(float angulo, Mat4* out) {
    float s = sinf(angulo);
    float c = cosf(angulo);
    
    MatIdentidad(out);
    out->m[0] = c;  out->m[2] = -s;
    out->m[8] = s;  out->m[10] = c;
}

// Matriz modelo (cubo centrado en origen)
void MatrizModelo(float tiempo, Mat4* out) {
    MatrizRotacionY(tiempo * ROT_VEL, out);
}

// Matriz MVP completa
void MatrizMVP(float tiempo, float aspecto, Mat4* out) {
    Mat4 modelo, vista, proj;
    
    MatrizModelo(tiempo, &modelo);
    MatrizVista(CAM_DIST, &vista);
    MatrizProyeccion(FOV_GRADOS, aspecto, 0.1f, 100.0f, &proj);
    
    // Vista * Modelo
    Mat4 view_model;
    MultiplicarMat(&view_model, &vista, &modelo);
    
    // Proj * (Vista * Modelo) = MVP
    MultiplicarMat(out, &proj, &view_model);
}

// --- SHADERS ---
static SDL_GPUShader* CrearShader(SDL_GPUDevice* gpu, const char* ruta, SDL_GPUShaderStage stage, uint32_t num_uniforms) {
    size_t size = 0;
    void* code = SDL_LoadFile(ruta, &size);
    if (!code) {
        fprintf(stderr, "Error: No se pudo cargar shader %s\n", ruta);
        return NULL;
    }
    
    SDL_GPUShaderCreateInfo info = {
        .code = code,
        .code_size = size,
        .entrypoint = "main",
        .format = SDL_GPU_SHADERFORMAT_SPIRV,
        .stage = stage,
        .num_uniform_buffers = num_uniforms
    };
    
    SDL_GPUShader* shader = SDL_CreateGPUShader(gpu, &info);
    SDL_free(code);
    
    if (!shader) {
        fprintf(stderr, "Error: No se pudo crear shader %s\n", ruta);
    }
    
    CheckSDLError("CrearShader");
    return shader;
}

// --- VERTICES DEL CUBO ---
static float vertices[] = {
    // Cara frontal (roja)
    -1.0f, -1.0f,  1.0f,  1.0f, 0.0f, 0.0f,
     1.0f, -1.0f,  1.0f,  1.0f, 0.0f, 0.0f,
     1.0f,  1.0f,  1.0f,  1.0f, 0.0f, 0.0f,
    -1.0f, -1.0f,  1.0f,  1.0f, 0.0f, 0.0f,
     1.0f,  1.0f,  1.0f,  1.0f, 0.0f, 0.0f,
    -1.0f,  1.0f,  1.0f,  1.0f, 0.0f, 0.0f,
    
    // Cara trasera (verde)
    -1.0f, -1.0f, -1.0f,  0.0f, 1.0f, 0.0f,
     1.0f,  1.0f, -1.0f,  0.0f, 1.0f, 0.0f,
     1.0f, -1.0f, -1.0f,  0.0f, 1.0f, 0.0f,
    -1.0f, -1.0f, -1.0f,  0.0f, 1.0f, 0.0f,
    -1.0f,  1.0f, -1.0f,  0.0f, 1.0f, 0.0f,
     1.0f,  1.0f, -1.0f,  0.0f, 1.0f, 0.0f,
    
    // Cara superior (azul)
    -1.0f,  1.0f,  1.0f,  0.0f, 0.0f, 1.0f,
     1.0f,  1.0f,  1.0f,  0.0f, 0.0f, 1.0f,
     1.0f,  1.0f, -1.0f,  0.0f, 0.0f, 1.0f,
    -1.0f,  1.0f,  1.0f,  0.0f, 0.0f, 1.0f,
     1.0f,  1.0f, -1.0f,  0.0f, 0.0f, 1.0f,
    -1.0f,  1.0f, -1.0f,  0.0f, 0.0f, 1.0f,
    
    // Cara inferior (amarilla)
    -1.0f, -1.0f,  1.0f,  1.0f, 1.0f, 0.0f,
     1.0f, -1.0f, -1.0f,  1.0f, 1.0f, 0.0f,
     1.0f, -1.0f,  1.0f,  1.0f, 1.0f, 0.0f,
    -1.0f, -1.0f,  1.0f,  1.0f, 1.0f, 0.0f,
    -1.0f, -1.0f, -1.0f,  1.0f, 1.0f, 0.0f,
     1.0f, -1.0f, -1.0f,  1.0f, 1.0f, 0.0f,
    
    // Cara derecha (cian)
     1.0f, -1.0f,  1.0f,  0.0f, 1.0f, 1.0f,
     1.0f, -1.0f, -1.0f,  0.0f, 1.0f, 1.0f,
     1.0f,  1.0f, -1.0f,  0.0f, 1.0f, 1.0f,
     1.0f, -1.0f,  1.0f,  0.0f, 1.0f, 1.0f,
     1.0f,  1.0f, -1.0f,  0.0f, 1.0f, 1.0f,
     1.0f,  1.0f,  1.0f,  0.0f, 1.0f, 1.0f,
    
    // Cara izquierda (magenta)
    -1.0f, -1.0f,  1.0f,  1.0f, 0.0f, 1.0f,
    -1.0f,  1.0f, -1.0f,  1.0f, 0.0f, 1.0f,
    -1.0f, -1.0f, -1.0f,  1.0f, 0.0f, 1.0f,
    -1.0f, -1.0f,  1.0f,  1.0f, 0.0f, 1.0f,
    -1.0f,  1.0f,  1.0f,  1.0f, 0.0f, 1.0f,
    -1.0f,  1.0f, -1.0f,  1.0f, 0.0f, 1.0f
};

// --- DEBUG MATRIX ---
void DebugMatrix(const char* nombre, const Mat4* m) {
    printf("\n=== %s ===\n", nombre);
    for (int i = 0; i < 4; i++) {
        printf("[ ");
        for (int j = 0; j < 4; j++) {
            printf("% 8.4f ", m->m[i * 4 + j]);
        }
        printf("]\n");
    }
}

// --- MAIN ---
int main(int argc, char **argv) {
    printf("=== INICIANDO CUBO 3D SDL3 ===\n");
    
    // Inicializar SDL
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        fprintf(stderr, "Error SDL_Init: %s\n", SDL_GetError());
        return 1;
    }
    
    printf("SDL3 inicializado correctamente\n");
    
    // Crear ventana
    SDL_Window* window = SDL_CreateWindow("Cubo 3D - Compatible Intel/NVIDIA", 
                                         WINDOW_W, WINDOW_H, 
                                         SDL_WINDOW_RESIZABLE);
    if (!window) {
        fprintf(stderr, "Error al crear ventana: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    printf("Ventana creada: %dx%d\n", WINDOW_W, WINDOW_H);
    
    // Crear dispositivo GPU
    SDL_GPUDevice* gpu = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, NULL);
    if (!gpu) {
        fprintf(stderr, "Error al crear dispositivo GPU: %s\n", SDL_GetError());
        printf("Intentando sin SPIR-V...\n");
        gpu = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_MSL, true, NULL);
        if (!gpu) {
            fprintf(stderr, "Error crítico: No se pudo crear dispositivo GPU\n");
            SDL_DestroyWindow(window);
            SDL_Quit();
            return 1;
        }
    }
    printf("GPU creada exitosamente\n");
    
    // Vincular ventana con GPU
    if (!SDL_ClaimWindowForGPUDevice(gpu, window)) {
        fprintf(stderr, "Advertencia: No se pudo vincular ventana con GPU: %s\n", SDL_GetError());
    }
    
    // Cargar shaders
    printf("Cargando shaders...\n");
    SDL_GPUShader* vsh = CrearShader(gpu, "src/shaders/simple.vert.spv", 
                                    SDL_GPU_SHADERSTAGE_VERTEX, 1);
    if (!vsh) {
        fprintf(stderr, "ERROR: Vertex shader no cargado\n");
        goto cleanup;
    }
    
    SDL_GPUShader* fsh = CrearShader(gpu, "src/shaders/simple.frag.spv", 
                                    SDL_GPU_SHADERSTAGE_FRAGMENT, 0);
    if (!fsh) {
        fprintf(stderr, "ERROR: Fragment shader no cargado\n");
        SDL_ReleaseGPUShader(gpu, vsh);
        goto cleanup;
    }
    printf("Shaders cargados correctamente\n");
    
    // Crear pipeline de gráficos
    printf("Creando pipeline...\n");
    SDL_GPUGraphicsPipeline* pipeline = SDL_CreateGPUGraphicsPipeline(gpu, 
        &(SDL_GPUGraphicsPipelineCreateInfo){
            .target_info = {
                .num_color_targets = 1,
                .color_target_descriptions = (SDL_GPUColorTargetDescription[]){
                    { 
                        .format = SDL_GetGPUSwapchainTextureFormat(gpu, window),
                        .blend_state = {
                            .enable_blend = false,
                            .src_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE,
                            .dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ZERO
                        }
                    }
                },
                .has_depth_stencil_target = false,  // Temporalmente sin depth para debug
            },
            .depth_stencil_state = {
                .enable_depth_test = false,
                .enable_depth_write = false,
                .compare_op = SDL_GPU_COMPAREOP_LESS
            },
            .rasterizer_state = {
                .cull_mode = SDL_GPU_CULLMODE_NONE,
                .front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE,
                .fill_mode = SDL_GPU_FILLMODE_FILL,
                .enable_depth_clip = true
            },
            .vertex_shader = vsh,
            .fragment_shader = fsh,
            .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
            .vertex_input_state = {
                .num_vertex_buffers = 1,
                .vertex_buffer_descriptions = (SDL_GPUVertexBufferDescription[]){
                    { 
                        .slot = 0, 
                        .pitch = sizeof(float) * 6,
                        .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX
                    }
                },
                .num_vertex_attributes = 2,
                .vertex_attributes = (SDL_GPUVertexAttribute[]){
                    { 
                        .location = 0,
                        .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
                        .offset = 0
                    },
                    { 
                        .location = 1,
                        .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
                        .offset = sizeof(float) * 3
                    }
                }
            }
        });
    
    CheckSDLError("Crear pipeline");
    
    if (!pipeline) {
        fprintf(stderr, "ERROR: No se pudo crear pipeline\n");
        SDL_ReleaseGPUShader(gpu, vsh);
        SDL_ReleaseGPUShader(gpu, fsh);
        goto cleanup;
    }
    printf("Pipeline creado exitosamente\n");
    
    // Liberar shaders (ya no se necesitan)
    SDL_ReleaseGPUShader(gpu, vsh);
    SDL_ReleaseGPUShader(gpu, fsh);
    
    // Crear VBO
    printf("Creando VBO...\n");
    SDL_GPUBuffer* vbo = SDL_CreateGPUBuffer(gpu, 
        &(SDL_GPUBufferCreateInfo){
            .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
            .size = sizeof(vertices)
        });
    
    if (!vbo) {
        fprintf(stderr, "ERROR: No se pudo crear VBO\n");
        goto cleanup;
    }
    
    // Crear buffer de transferencia y subir datos
    printf("Subiendo datos de vértices...\n");
    SDL_GPUTransferBuffer* upload_buffer = SDL_CreateGPUTransferBuffer(gpu,
        &(SDL_GPUTransferBufferCreateInfo){
            .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
            .size = sizeof(vertices)
        });
    
    if (!upload_buffer) {
        fprintf(stderr, "ERROR: No se pudo crear buffer de upload\n");
        SDL_ReleaseGPUBuffer(gpu, vbo);
        goto cleanup;
    }
    
    // Copiar datos
    void* upload_ptr = SDL_MapGPUTransferBuffer(gpu, upload_buffer, false);
    if (upload_ptr) {
        memcpy(upload_ptr, vertices, sizeof(vertices));
        SDL_UnmapGPUTransferBuffer(gpu, upload_buffer);
    }
    
    // Transferir al VBO
    SDL_GPUCommandBuffer* copy_cmd = SDL_AcquireGPUCommandBuffer(gpu);
    if (copy_cmd) {
        SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(copy_cmd);
        SDL_UploadToGPUBuffer(copy_pass,
            &(SDL_GPUTransferBufferLocation){.transfer_buffer = upload_buffer},
            &(SDL_GPUBufferRegion){.buffer = vbo, .size = sizeof(vertices)},
            false);
        SDL_EndGPUCopyPass(copy_pass);
        SDL_SubmitGPUCommandBuffer(copy_cmd);
    }
    SDL_ReleaseGPUTransferBuffer(gpu, upload_buffer);
    printf("Vértices cargados en GPU (%zu bytes)\n", sizeof(vertices));
    
    // Variables del bucle principal
    bool running = true;
    int frame_counter = 0;
    printf("\n=== INICIANDO BUCLE PRINCIPAL ===\n");
    
    while (running) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_EVENT_QUIT) {
                running = false;
            }
            if (ev.type == SDL_EVENT_KEY_DOWN && ev.key.key == SDLK_ESCAPE) {
                running = false;
            }
        }
        
        // Obtener tamaño de ventana
        int width, height;
        SDL_GetWindowSizeInPixels(window, &width, &height);
        if (width <= 0 || height <= 0) {
            continue;
        }
        
        // DEBUG: Información del frame
        frame_counter++;
        if (frame_counter % 60 == 0) {
            printf("Frame: %d, Ventana: %dx%d\n", frame_counter, width, height);
        }
        
        // Calcular matriz MVP
        Mat4 mvp;
        float tiempo = (float)SDL_GetTicks() / 1000.0f;
        float aspecto = (float)width / (float)height;
        
        if (frame_counter == 1) {  // Solo mostrar en el primer frame para no saturar
            printf("\n=== DEBUG PRIMER FRAME ===\n");
            printf("Tiempo: %.3f, Aspecto: %.3f\n", tiempo, aspecto);
        }
        
        MatrizMVP(tiempo, aspecto, &mvp);
        
        if (frame_counter == 1) {
            DebugMatrix("Matriz MVP", &mvp);
            printf("Tamaño matriz: %zu bytes\n", sizeof(Mat4));
        }
        
        // Comenzar renderizado
        SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(gpu);
        if (!cmd) {
            fprintf(stderr, "ERROR: No se pudo adquirir command buffer\n");
            continue;
        }
        
        SDL_GPUTexture* swapchain_texture = NULL;
        
        if (SDL_AcquireGPUSwapchainTexture(cmd, window, &swapchain_texture, NULL, NULL) && swapchain_texture) {
            // Empujar uniformes (push constants)
            if (frame_counter == 1) {
                printf("Pusheando uniformes...\n");
            }
            
            SDL_PushGPUVertexUniformData(cmd, 0, &mvp, sizeof(Mat4));
            
            // Configurar render pass
            SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(cmd,
                &(SDL_GPUColorTargetInfo){
                    .texture = swapchain_texture,
                    .clear_color = {0.2f, 0.2f, 0.3f, 1.0f},  // Fondo morado
                    .load_op = SDL_GPU_LOADOP_CLEAR,
                    .store_op = SDL_GPU_STOREOP_STORE
                }, 1,
                NULL  // Sin depth/stencil por ahora
                );
            
            if (!pass) {
                fprintf(stderr, "ERROR: No se pudo crear render pass\n");
                CheckSDLError("SDL_BeginGPURenderPass");
            } else {
                // Vincular pipeline y dibujar
                SDL_BindGPUGraphicsPipeline(pass, pipeline);
                SDL_BindGPUVertexBuffers(pass, 0, &(SDL_GPUBufferBinding){.buffer = vbo}, 1);
                
                if (frame_counter == 1) {
                    printf("Dibujando 36 vértices...\n");
                }
                
                SDL_DrawGPUPrimitives(pass, 36, 1, 0, 0);
                SDL_EndGPURenderPass(pass);
                
                if (frame_counter == 1) {
                    printf("Render pass completado\n");
                }
            }
            
            SDL_SubmitGPUCommandBuffer(cmd);
        } else {
            SDL_CancelGPUCommandBuffer(cmd);
            fprintf(stderr, "ERROR: No se pudo adquirir textura del swapchain\n");
        }
        
        // Pequeña pausa
        SDL_Delay(10);
    }
    
    printf("\n=== CERRANDO APLICACIÓN ===\n");
    
cleanup:
    // Limpieza
    SDL_WaitForGPUIdle(gpu);
    
    if (vbo) {
        SDL_ReleaseGPUBuffer(gpu, vbo);
    }
    if (pipeline) {
        SDL_ReleaseGPUGraphicsPipeline(gpu, pipeline);
    }
    if (gpu) {
        SDL_DestroyGPUDevice(gpu);
    }
    if (window) {
        SDL_DestroyWindow(window);
    }
    
    SDL_Quit();
    printf("Aplicación terminada correctamente\n");
    return 0;
}