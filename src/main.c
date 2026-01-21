// Librerías SDL3
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

// Mis librerías
#include "math_3d.h"
#include "utils.h"
#include "ui.h"
#include "gestion_memoria.h"
#include "modelo.h"

// Configuraciones de pantalla
#define VENTANA_ANCHO 800
#define VENTANA_ALTO 600

// Configuraciones de la camara
#define DISTANCIA_CAMARA 1.0
#define ZOOM 1.0

// Configuracion memoria
#define ARENA_SIZE_MB 128

// --- FUNCIONES AUXILIARES GPU ---
SDL_GPUShader *crear_shader_gpu(SDL_GPUDevice *device, const char *ruta, SDL_GPUShaderStage stage)
{
    size_t tamano;
    void *codigo = leer_archivo_binario(ruta, &tamano);
    if (!codigo)
        return NULL;

    SDL_GPUShaderCreateInfo info = {
        .code = codigo,
        .code_size = tamano,
        .entrypoint = "main",
        .format = SDL_GPU_SHADERFORMAT_SPIRV,
        .stage = stage};

    SDL_GPUShader *shader = SDL_CreateGPUShader(device, &info);
    free(codigo);
    return shader;
}

// --- MAIN ---
int main(int argc, char *argv[])
{
    if (!SDL_Init(SDL_INIT_VIDEO))
        return 1;

    ui_init();

    // 1. GESTIÓN DE MEMORIA
    Arena arena_global;
    arena_inicializar(&arena_global, 20 * 1024 * 1024);
    Arena arena_escena;
    arena_inicializar(&arena_escena, (ARENA_SIZE_MB - 20) * 1024 * 1024);

    // 2. INICIALIZACIÓN GPU Y VENTANA
    SDL_Window *window = SDL_CreateWindow("Motor 3D - GPU Hibrido", VENTANA_ANCHO, VENTANA_ALTO, 0);
    if (!window)
        return 1;

    SDL_GPUDevice *gpu = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, NULL);
    if (!gpu)
    {
        SDL_Log("Error: No se pudo crear el dispositivo GPU");
        return 1;
    }
    SDL_ClaimWindowForGPUDevice(gpu, window);

    // Renderer para UI y depuración
    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);

    // 3. CARGA DE SHADERS
    SDL_GPUShader *shader_vert = crear_shader_gpu(gpu, "src/shaders/simple.vert.spv", SDL_GPU_SHADERSTAGE_VERTEX);
    SDL_GPUShader *shader_frag = crear_shader_gpu(gpu, "src/shaders/simple.frag.spv", SDL_GPU_SHADERSTAGE_FRAGMENT);

    // --- NUEVO: CREACIÓN DEL PIPELINE ---
    SDL_GPUGraphicsPipeline *pipeline = NULL;
    if (shader_vert && shader_frag)
    {
        SDL_GPUGraphicsPipelineCreateInfo pipeline_info = {
            .vertex_shader = shader_vert,
            .fragment_shader = shader_frag,
            .vertex_input_state = (SDL_GPUVertexInputState){
                .num_vertex_buffers = 1,
                .vertex_buffer_descriptions = (SDL_GPUVertexBufferDescription[]){{.slot = 0,
                                                                                  .pitch = sizeof(float) * 3,
                                                                                  .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX}},
                .num_vertex_attributes = 1,
                .vertex_attributes = (SDL_GPUVertexAttribute[]){{.buffer_slot = 0,
                                                                 .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
                                                                 .location = 0,
                                                                 .offset = 0}}},
            .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
            .target_info = {.num_color_targets = 1, .color_target_descriptions = (SDL_GPUColorTargetDescription[]){{.format = SDL_GetGPUSwapchainTextureFormat(gpu, window)}}}};
        pipeline = SDL_CreateGPUGraphicsPipeline(gpu, &pipeline_info);
    }

    if (pipeline)
        SDL_Log("GPU: Pipeline creado con éxito.");
    else
        SDL_Log("Error: No se pudo crear el pipeline.");

    // 4. ESTADO DEL MOTOR
    Modelo *modelo_actual = NULL;
    float escala = (VENTANA_ANCHO / 2.0f) * ZOOM;
    float angulo = 0.0f;
    float velocidad_giro = 20.0f;
    float *z_buffer = inicializar_zbuffer(&arena_global, VENTANA_ANCHO, VENTANA_ALTO);

    // 5. VARIABLES DE TIEMPO
    Uint64 tiempo_ahora = SDL_GetTicks();
    Uint64 tiempo_ultimo = 0;
    Uint64 tiempo_anterior_fps = SDL_GetTicks();
    Uint64 frames_contados = 0;
    float fps_actuales = 0;
    float dt = 0;
    char texto_fps[64] = "Iniciando...";

    // 6. UI Y BOTONES
    Uint64 ultimo_clic = 0;
    const Uint64 COOLDOWN_BOTON = 200;
    bool bool_vsync = true;

    FunctionCambioVsync params_vsync = {renderer, &bool_vsync, &ultimo_clic, COOLDOWN_BOTON};
    Boton btn_vsync = {20, 50, 140, 30, {100, 100, 100, 255}, "VSYNC ON/OFF", accion_cambiar_vsync, &params_vsync};

    FunctionCargarModelo params_modelo = {&modelo_actual, &ultimo_clic, COOLDOWN_BOTON, &arena_escena, gpu};
    Boton btn_cambio = {20, 90, 140, 30, {100, 100, 100, 255}, "CARGAR MODELO", accion_cargar_modelo, &params_modelo};

    SDL_SetRenderVSync(renderer, bool_vsync);

    // --- BUCLE PRINCIPAL ---
    bool corriendo = true;
    SDL_Event ev;

    while (corriendo)
    {
        ui_comenzar_frame();
        while (SDL_PollEvent(&ev))
        {
            if (ev.type == SDL_EVENT_QUIT)
                corriendo = false;
        }

        tiempo_ultimo = tiempo_ahora;
        tiempo_ahora = SDL_GetTicks();
        dt = (tiempo_ahora - tiempo_ultimo) / 1000.0f;

        // --- RENDERIZADO GPU ---
        SDL_GPUCommandBuffer *cmd = SDL_AcquireGPUCommandBuffer(gpu);
        SDL_GPUTexture *swapchain_tex;
        if (SDL_AcquireGPUSwapchainTexture(cmd, window, &swapchain_tex, NULL, NULL))
        {
            SDL_GPUColorTargetInfo color_info = {0};
            color_info.texture = swapchain_tex;
            color_info.clear_color = (SDL_FColor){0.05f, 0.05f, 0.05f, 1.0f};
            color_info.load_op = SDL_GPU_LOADOP_CLEAR;
            color_info.store_op = SDL_GPU_STOREOP_STORE;

            SDL_GPURenderPass *render_pass = SDL_BeginGPURenderPass(cmd, &color_info, 1, NULL);

            // Solo dibujamos si hay un modelo y tiene buffer en la GPU
            if (modelo_actual != NULL && modelo_actual->vertex_buffer_gpu != NULL && pipeline != NULL)
            {
                SDL_BindGPUGraphicsPipeline(render_pass, pipeline);
                SDL_BindGPUVertexBuffers(render_pass, 0, &(SDL_GPUBufferBinding){.buffer = modelo_actual->vertex_buffer_gpu, .offset = 0}, 1);
                SDL_DrawGPUPrimitives(render_pass, modelo_actual->n_puntos, 1, 0, 0);
            }
            SDL_EndGPURenderPass(render_pass);
        }
        SDL_SubmitGPUCommandBuffer(cmd);

        // --- RENDERIZADO CPU (Overlay de UI) ---
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);

        limpiar_zbuffer(z_buffer, VENTANA_ANCHO, VENTANA_ALTO);

        if (modelo_actual != NULL)
        {
            pintar_modelo(modelo_actual, renderer, z_buffer, angulo, DISTANCIA_CAMARA, VENTANA_ANCHO, VENTANA_ALTO, escala);
        }

        calcular_frames(&fps_actuales, &frames_contados, texto_fps, sizeof(texto_fps), &tiempo_anterior_fps);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDebugText(renderer, 10, 10, texto_fps);
        ui_dibujar_boton(renderer, &btn_vsync, btn_vsync.params);
        ui_dibujar_boton(renderer, &btn_cambio, btn_cambio.params);

        SDL_RenderPresent(renderer);

        // --- LÓGICA ---
        angulo += velocidad_giro * dt;
        gestionar_cursor_raton();
    }

    // --- LIMPIEZA ---
    if (pipeline)
        SDL_ReleaseGPUGraphicsPipeline(gpu, pipeline);
    if (shader_vert)
        SDL_ReleaseGPUShader(gpu, shader_vert);
    if (shader_frag)
        SDL_ReleaseGPUShader(gpu, shader_frag);

    SDL_ReleaseWindowFromGPUDevice(gpu, window);
    SDL_DestroyGPUDevice(gpu);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    free(arena_global.base);
    free(arena_escena.base);

    return 0;
}