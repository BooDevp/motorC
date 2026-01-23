#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// --- AJUSTES ---
#define WINDOW_W 800
#define WINDOW_H 600
#define FOV_GRADOS 60.0f
#define CAM_DIST 4.0f
#define ROT_VEL 1.0f

// --- ESTRUCTURAS ---
typedef struct
{
    float m[16];
} Mat4;

// --- DEBUG ---
static void CheckSDLError(const char *context)
{
    const char *error = SDL_GetError();
    if (error && *error)
    {
        fprintf(stderr, "%s: %s\n", context, error);
        SDL_ClearError();
    }
}

// --- MATEMÁTICAS ---
void MatIdentidad(Mat4 *m)
{
    memset(m->m, 0, sizeof(float) * 16);
    m->m[0] = 1.0f;
    m->m[5] = 1.0f;
    m->m[10] = 1.0f;
    m->m[15] = 1.0f;
}

void MultiplicarMat(Mat4 *out, const Mat4 *a, const Mat4 *b)
{
    Mat4 r;
    for (int col = 0; col < 4; col++)
    {
        for (int row = 0; row < 4; row++)
        {
            r.m[col * 4 + row] =
                a->m[0 * 4 + row] * b->m[col * 4 + 0] +
                a->m[1 * 4 + row] * b->m[col * 4 + 1] +
                a->m[2 * 4 + row] * b->m[col * 4 + 2] +
                a->m[3 * 4 + row] * b->m[col * 4 + 3];
        }
    }
    *out = r;
}

// Matriz de proyección perspectiva CORREGIDA para NVIDIA
void MatrizProyeccion(float fov_grados, float aspecto, float near, float far, Mat4 *out)
{
    float fov_rad = fov_grados * 0.5f * (3.14159265f / 180.0f);
    float f = 1.0f / tanf(fov_rad);

    // Matriz de proyección perspectiva estándar (Vulkan, Z [0, 1])
    // IMPORTANTE: NVIDIA necesita el flip en Y
    Mat4 proj = {0};
    proj.m[0] = f / aspecto;
    proj.m[5] = -f; // FLIP Y para NVIDIA (Vulkan tiene Y hacia abajo)
    proj.m[10] = far / (far - near);
    proj.m[11] = 1.0f; // Para proyección perspectiva
    proj.m[14] = -(far * near) / (far - near);

    // Corrección para mapear Z de [-1,1] a [0,1] (Vulkan)
    Mat4 clip_correction = {0};
    clip_correction.m[0] = 1.0f;
    clip_correction.m[5] = 1.0f;
    clip_correction.m[10] = 0.5f;
    clip_correction.m[14] = 0.5f;
    clip_correction.m[15] = 1.0f;

    MultiplicarMat(out, &clip_correction, &proj);
}

// Matriz de vista (cámara en Z negativo mirando hacia +Z)
void MatrizVista(float distancia, Mat4 *out)
{
    MatIdentidad(out);
    out->m[14] = -distancia;
}

// Matriz de rotación en Y
void MatrizRotacionY(float angulo, Mat4 *out)
{
    float s = sinf(angulo);
    float c = cosf(angulo);

    MatIdentidad(out);
    out->m[0] = c;
    out->m[2] = s; // CAMBIO: positivo s para correcta orientación
    out->m[8] = -s;
    out->m[10] = c; // CAMBIO: negativo s
}

// Matriz modelo (cubo centrado en origen)
void MatrizModelo(float tiempo, Mat4 *out)
{
    MatrizRotacionY(tiempo * ROT_VEL, out);
}

// Matriz MVP completa
void MatrizMVP(float tiempo, float aspecto, Mat4 *out)
{
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
static SDL_GPUShader *CrearShader(SDL_GPUDevice *gpu, const char *ruta, SDL_GPUShaderStage stage, uint32_t num_uniforms)
{
    size_t size = 0;
    void *code = SDL_LoadFile(ruta, &size);
    if (!code)
    {
        fprintf(stderr, "Error: No se pudo cargar shader %s\n", ruta);
        return NULL;
    }

    SDL_GPUShaderCreateInfo info = {
        .code = code,
        .code_size = size,
        .entrypoint = "main",
        .format = SDL_GPU_SHADERFORMAT_SPIRV,
        .stage = stage,
        .num_uniform_buffers = num_uniforms};

    SDL_GPUShader *shader = SDL_CreateGPUShader(gpu, &info);
    SDL_free(code);

    if (!shader)
    {
        fprintf(stderr, "Error: No se pudo crear shader %s\n", ruta);
    }

    CheckSDLError("CrearShader");
    return shader;
}

// --- VERTICES DEL CUBO ---
static float vertices[] = {
    // Cara frontal (roja)
    -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
    1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
    1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
    -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
    1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
    -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,

    // Cara trasera (verde)
    -1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    -1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,

    // Cara superior (azul)
    -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
    1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
    1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f,
    -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
    1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f,
    -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f,

    // Cara inferior (amarilla)
    -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
    1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 0.0f,
    1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
    -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
    -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 0.0f,
    1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 0.0f,

    // Cara derecha (cian)
    1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
    1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 1.0f,
    1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 1.0f,
    1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
    1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f,

    // Cara izquierda (magenta)
    -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
    -1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f,
    -1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 1.0f,
    -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
    -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
    -1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f};

// --- DEBUG ---
void DebugMatrix(const char *nombre, const Mat4 *m)
{
    printf("\n=== %s ===\n", nombre);
    for (int i = 0; i < 4; i++)
    {
        printf("[ ");
        for (int j = 0; j < 4; j++)
        {
            printf("% 8.4f ", m->m[i * 4 + j]);
        }
        printf("]\n");
    }
}

// --- MAIN ---
int main(int argc, char **argv)
{
    printf("=== CUBO 3D SDL3 - COMPATIBLE INTEL/NVIDIA ===\n");

    // Detectar GPU
    const char *gpu_vendor = getenv("GPU_VENDOR");
    if (gpu_vendor)
        printf("GPU forzada: %s\n", gpu_vendor);

    // Inicializar SDL
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
    {
        fprintf(stderr, "Error SDL_Init: %s\n", SDL_GetError());
        return 1;
    }
    printf("SDL3 inicializado\n");

    // Crear ventana
    SDL_Window *window = SDL_CreateWindow("Cubo 3D - Intel/NVIDIA",
                                          WINDOW_W, WINDOW_H,
                                          SDL_WINDOW_RESIZABLE);
    if (!window)
    {
        fprintf(stderr, "Error al crear ventana: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    printf("Ventana: %dx%d\n", WINDOW_W, WINDOW_H);

    // Crear dispositivo GPU con configuración para NVIDIA
    SDL_GPUDevice *gpu = NULL;

    // Primero intentar con SPIR-V (para Intel)
    gpu = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, NULL);
    if (!gpu)
    {
        printf("SPIR-V falló, intentando sin formato específico...\n");
        gpu = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_MSL, true, NULL);
        if (!gpu)
        {
            fprintf(stderr, "Error crítico: No se pudo crear dispositivo GPU\n");
            SDL_DestroyWindow(window);
            SDL_Quit();
            return 1;
        }
        printf("GPU creada sin SPIR-V (modo compatibilidad)\n");
    }
    else
    {
        printf("GPU creada con SPIR-V\n");
    }

    // Vincular ventana
    if (!SDL_ClaimWindowForGPUDevice(gpu, window))
    {
        printf("Advertencia: No se pudo vincular ventana con GPU\n");
        CheckSDLError("SDL_ClaimWindowForGPUDevice");
    }

    // Cargar shaders
    printf("Cargando shaders...\n");
    SDL_GPUShader *vsh = CrearShader(gpu, "src/shaders/simple.vert.spv",
                                     SDL_GPU_SHADERSTAGE_VERTEX, 1);
    if (!vsh)
    {
        fprintf(stderr, "ERROR: Vertex shader\n");
        goto cleanup;
    }

    SDL_GPUShader *fsh = CrearShader(gpu, "src/shaders/simple.frag.spv",
                                     SDL_GPU_SHADERSTAGE_FRAGMENT, 0);
    if (!fsh)
    {
        fprintf(stderr, "ERROR: Fragment shader\n");
        SDL_ReleaseGPUShader(gpu, vsh);
        goto cleanup;
    }
    printf("Shaders cargados\n");

    // Configuración del pipeline para NVIDIA
    printf("Creando pipeline...\n");

    // Primero verificar el formato del swapchain
    SDL_TimeFormat swapchain_format = SDL_GetGPUSwapchainTextureFormat(gpu, window);
    printf("Formato swapchain: %u\n", swapchain_format);

    // Crear pipeline CON depth testing para NVIDIA
    SDL_GPUGraphicsPipeline *pipeline = SDL_CreateGPUGraphicsPipeline(gpu,
                                                                      &(SDL_GPUGraphicsPipelineCreateInfo){
                                                                          .target_info = {
                                                                              .num_color_targets = 1,
                                                                              .color_target_descriptions = (SDL_GPUColorTargetDescription[]){
                                                                                  {.format = swapchain_format,
                                                                                   .blend_state = {
                                                                                       .enable_blend = false,
                                                                                       .src_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE,
                                                                                       .dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ZERO}}},
                                                                              .has_depth_stencil_target = true,
                                                                              .depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT, // NVIDIA prefiere D32
                                                                          },
                                                                          .depth_stencil_state = {
                                                                              .enable_depth_test = true,
                                                                              .enable_depth_write = true,
                                                                              .compare_op = SDL_GPU_COMPAREOP_LESS, // LESS (no LESS_OR_EQUAL) para NVIDIA
                                                                          },
                                                                          .rasterizer_state = {
                                                                              .cull_mode = SDL_GPU_CULLMODE_NONE,        // Desactivar culling para ver todas las caras
                                                                              .front_face = SDL_GPU_FRONTFACE_CLOCKWISE, // IMPORTANTE: Clockwise para NVIDIA/Vulkan
                                                                              .fill_mode = SDL_GPU_FILLMODE_FILL,
                                                                              .enable_depth_clip = true,
                                                                          },
                                                                          .vertex_shader = vsh,
                                                                          .fragment_shader = fsh,
                                                                          .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
                                                                          .vertex_input_state = {.num_vertex_buffers = 1, .vertex_buffer_descriptions = (SDL_GPUVertexBufferDescription[]){{.slot = 0,
                                                                                                                                                                                            .pitch = sizeof(float) * 6, // 3 pos + 3 color
                                                                                                                                                                                            .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX}},
                                                                                                 .num_vertex_attributes = 2,
                                                                                                 .vertex_attributes = (SDL_GPUVertexAttribute[]){{.location = 0, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, .offset = 0}, {.location = 1, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, .offset = sizeof(float) * 3}}}});

    CheckSDLError("Crear pipeline");

    if (!pipeline)
    {
        fprintf(stderr, "ERROR: Pipeline\n");
        SDL_ReleaseGPUShader(gpu, vsh);
        SDL_ReleaseGPUShader(gpu, fsh);
        goto cleanup;
    }
    printf("Pipeline creado (Depth: D32_FLOAT, Cull: NONE, Front: CLOCKWISE)\n");

    // Liberar shaders
    SDL_ReleaseGPUShader(gpu, vsh);
    SDL_ReleaseGPUShader(gpu, fsh);

    // Crear VBO
    printf("Creando VBO...\n");
    SDL_GPUBuffer *vbo = SDL_CreateGPUBuffer(gpu,
                                             &(SDL_GPUBufferCreateInfo){
                                                 .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
                                                 .size = sizeof(vertices)});

    if (!vbo)
    {
        fprintf(stderr, "ERROR: VBO\n");
        goto cleanup;
    }

    // Subir datos al VBO
    printf("Subiendo vértices (%zu bytes)...\n", sizeof(vertices));
    SDL_GPUTransferBuffer *upload_buffer = SDL_CreateGPUTransferBuffer(gpu,
                                                                       &(SDL_GPUTransferBufferCreateInfo){
                                                                           .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
                                                                           .size = sizeof(vertices)});

    if (!upload_buffer)
    {
        fprintf(stderr, "ERROR: Upload buffer\n");
        SDL_ReleaseGPUBuffer(gpu, vbo);
        goto cleanup;
    }

    void *upload_ptr = SDL_MapGPUTransferBuffer(gpu, upload_buffer, false);
    if (upload_ptr)
    {
        memcpy(upload_ptr, vertices, sizeof(vertices));
        SDL_UnmapGPUTransferBuffer(gpu, upload_buffer);
    }

    // Transferir al VBO
    SDL_GPUCommandBuffer *copy_cmd = SDL_AcquireGPUCommandBuffer(gpu);
    if (copy_cmd)
    {
        SDL_GPUCopyPass *copy_pass = SDL_BeginGPUCopyPass(copy_cmd);
        SDL_UploadToGPUBuffer(copy_pass,
                              &(SDL_GPUTransferBufferLocation){.transfer_buffer = upload_buffer},
                              &(SDL_GPUBufferRegion){.buffer = vbo, .size = sizeof(vertices)},
                              false);
        SDL_EndGPUCopyPass(copy_pass);
        SDL_SubmitGPUCommandBuffer(copy_cmd);
    }
    SDL_ReleaseGPUTransferBuffer(gpu, upload_buffer);
    printf("VBO listo\n");

    // Preparar textura de profundidad
    SDL_GPUTexture *depth_texture = NULL;
    uint32_t depth_width = 0, depth_height = 0;

    bool running = true;
    int frame_counter = 0;
    printf("\n=== INICIANDO RENDER ===\n");

    while (running)
    {
        SDL_Event ev;
        while (SDL_PollEvent(&ev))
        {
            if (ev.type == SDL_EVENT_QUIT)
                running = false;
            if (ev.type == SDL_EVENT_KEY_DOWN && ev.key.key == SDLK_ESCAPE)
                running = false;
            if (ev.type == SDL_EVENT_KEY_DOWN && ev.key.key == SDLK_F1)
            {
                // Debug con F1
                printf("Debug frame %d\n", frame_counter);
            }
        }

        int width, height;
        SDL_GetWindowSizeInPixels(window, &width, &height);
        if (width <= 0 || height <= 0)
            continue;

        frame_counter++;

        // Recrear textura de profundidad si cambia el tamaño
        if (!depth_texture || (uint32_t)width != depth_width || (uint32_t)height != depth_height)
        {
            SDL_WaitForGPUIdle(gpu);
            if (depth_texture)
                SDL_ReleaseGPUTexture(gpu, depth_texture);

            depth_width = (uint32_t)width;
            depth_height = (uint32_t)height;

            depth_texture = SDL_CreateGPUTexture(gpu,
                                                 &(SDL_GPUTextureCreateInfo){
                                                     .type = SDL_GPU_TEXTURETYPE_2D,
                                                     .format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT,
                                                     .width = depth_width,
                                                     .height = depth_height,
                                                     .layer_count_or_depth = 1,
                                                     .num_levels = 1,
                                                     .usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET});

            if (!depth_texture)
            {
                fprintf(stderr, "ERROR: Depth texture\n");
                continue;
            }

            if (frame_counter == 1)
            {
                printf("Depth texture: %ux%u (D32_FLOAT)\n", depth_width, depth_height);
            }
        }

        // Calcular matriz MVP
        Mat4 mvp;
        float tiempo = (float)SDL_GetTicks() / 1000.0f;
        float aspecto = (float)width / (float)height;

        if (frame_counter == 1)
        {
            printf("\n=== PRIMER FRAME DEBUG ===\n");
            printf("Tiempo: %.3f, Aspecto: %.3f\n", tiempo, aspecto);
        }

        MatrizMVP(tiempo, aspecto, &mvp);

        if (frame_counter == 1)
        {
            DebugMatrix("Matriz MVP", &mvp);
            printf("Enviando %zu bytes a uniform slot 0\n", sizeof(Mat4));
        }

        // Render
        SDL_GPUCommandBuffer *cmd = SDL_AcquireGPUCommandBuffer(gpu);
        if (!cmd)
        {
            fprintf(stderr, "ERROR: Command buffer\n");
            continue;
        }

        SDL_GPUTexture *swapchain_texture = NULL;

        if (SDL_AcquireGPUSwapchainTexture(cmd, window, &swapchain_texture, NULL, NULL) && swapchain_texture)
        {
            // Push uniformes ANTES del render pass
            SDL_PushGPUVertexUniformData(cmd, 0, &mvp, sizeof(Mat4));

            // Render pass CON depth
            SDL_GPURenderPass *pass = SDL_BeginGPURenderPass(cmd,
                                                             &(SDL_GPUColorTargetInfo){
                                                                 .texture = swapchain_texture,
                                                                 .clear_color = {0.2f, 0.2f, 0.3f, 1.0f},
                                                                 .load_op = SDL_GPU_LOADOP_CLEAR,
                                                                 .store_op = SDL_GPU_STOREOP_STORE},
                                                             1,
                                                             &(SDL_GPUDepthStencilTargetInfo){
                                                                 .texture = depth_texture,
                                                                 .clear_depth = 1.0f,
                                                                 .load_op = SDL_GPU_LOADOP_CLEAR,
                                                                 .store_op = SDL_GPU_STOREOP_DONT_CARE});

            if (!pass)
            {
                fprintf(stderr, "ERROR: Render pass\n");
                CheckSDLError("SDL_BeginGPURenderPass");
            }
            else
            {
                // Dibujar
                SDL_BindGPUGraphicsPipeline(pass, pipeline);
                SDL_BindGPUVertexBuffers(pass, 0, &(SDL_GPUBufferBinding){.buffer = vbo}, 1);
                SDL_DrawGPUPrimitives(pass, 36, 1, 0, 0);

                if (frame_counter == 1)
                {
                    printf("Draw: 36 vértices\n");
                }

                SDL_EndGPURenderPass(pass);

                if (frame_counter == 1)
                {
                    printf("Render completado\n");
                }
            }

            SDL_SubmitGPUCommandBuffer(cmd);
        }
        else
        {
            SDL_CancelGPUCommandBuffer(cmd);
            fprintf(stderr, "ERROR: Swapchain texture\n");
        }

        // FPS limit
        SDL_Delay(16);

        // Log cada segundo
        if (frame_counter % 60 == 0)
        {
            printf("Frame %d, %dx%d\n", frame_counter, width, height);
        }
    }

    printf("\n=== FINALIZANDO ===\n");

cleanup:
    SDL_WaitForGPUIdle(gpu);

    if (depth_texture)
        SDL_ReleaseGPUTexture(gpu, depth_texture);
    if (vbo)
        SDL_ReleaseGPUBuffer(gpu, vbo);
    if (pipeline)
        SDL_ReleaseGPUGraphicsPipeline(gpu, pipeline);
    if (gpu)
        SDL_DestroyGPUDevice(gpu);
    if (window)
        SDL_DestroyWindow(window);

    SDL_Quit();
    printf("Aplicación terminada\n");
    return 0;
}