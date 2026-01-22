#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdbool.h>

// --- ESTRUCTURAS ---

typedef struct { 
    float m[16]; 
} Mat4;

// --- FUNCIONES MATEMÁTICAS ---

void MatrizRotacionZ(float angulo, Mat4* out) {
    float s = SDL_sinf(angulo);
    float c = SDL_cosf(angulo);
    for(int i=0; i<16; i++) out->m[i] = 0;
    
    // Matriz de rotación estándar para el eje Z
    out->m[0] = c;  out->m[1] = -s;
    out->m[4] = s;  out->m[5] = c;
    out->m[10] = 1; out->m[15] = 1;
}

// --- FUNCIONES DE RECURSOS GPU ---

// Carga un archivo SPIR-V y crea el objeto Shader informando sus uniformes
static SDL_GPUShader* CrearShader(SDL_GPUDevice* gpu, const char* ruta, SDL_GPUShaderStage stage, uint32_t num_uniforms) {
    size_t size = 0;
    void* code = SDL_LoadFile(ruta, &size);
    if (!code) {
        SDL_Log("ERROR: No se pudo leer el archivo shader: %s", ruta);
        return NULL;
    }

    SDL_GPUShaderCreateInfo info = {
        .code = code,
        .code_size = size,
        .entrypoint = "main",
        .format = SDL_GPU_SHADERFORMAT_SPIRV,
        .stage = stage,
        .num_uniform_buffers = num_uniforms // Crucial para que coincida con el shader
    };

    SDL_GPUShader* shader = SDL_CreateGPUShader(gpu, &info);
    SDL_free(code);
    return shader;
}

// Configura el estado fijo de la GPU (Vertex layout, Shaders, Blending, etc.)
SDL_GPUGraphicsPipeline* CrearPipeline(SDL_GPUDevice* gpu, SDL_Window* window, SDL_GPUShader* vsh, SDL_GPUShader* fsh) {
    SDL_GPUGraphicsPipelineCreateInfo pipeInfo = {
        .target_info = {
            .num_color_targets = 1,
            .color_target_descriptions = (SDL_GPUColorTargetDescription[]){{
                .format = SDL_GetGPUSwapchainTextureFormat(gpu, window),
                .blend_state = { .enable_blend = false, .color_write_mask = 0xF }
            }}
        },
        .vertex_shader = vsh,
        .fragment_shader = fsh,
        .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
        .vertex_input_state = {
            .num_vertex_buffers = 1,
            .vertex_buffer_descriptions = (SDL_GPUVertexBufferDescription[]){{
                .slot = 0, .pitch = sizeof(float) * 6, .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX
            }},
            .num_vertex_attributes = 2,
            .vertex_attributes = (SDL_GPUVertexAttribute[]){
                { .location = 0, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, .offset = 0 }, // Posición
                { .location = 1, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, .offset = sizeof(float) * 3 } // Color
            }
        }
    };

    return SDL_CreateGPUGraphicsPipeline(gpu, &pipeInfo);
}

// --- LÓGICA PRINCIPAL ---
int main(int argc, char **argv) {
    if (!SDL_Init(SDL_INIT_VIDEO)) return 1;

    SDL_Window* window = SDL_CreateWindow("Motor SDL3 - Checkpoint Estable", 800, 600, 0);
    SDL_GPUDevice* gpu = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, NULL);
    if (!gpu || !window) return 1;

    SDL_ClaimWindowForGPUDevice(gpu, window);

    // 1. Carga de Shaders (Informamos que el Vertex tiene 1 buffer de uniformes)
    SDL_GPUShader* vsh = CrearShader(gpu, "src/shaders/simple.vert.spv", SDL_GPU_SHADERSTAGE_VERTEX, 1);
    SDL_GPUShader* fsh = CrearShader(gpu, "src/shaders/simple.frag.spv", SDL_GPU_SHADERSTAGE_FRAGMENT, 0);
    
    // 2. Creación del Pipeline
    SDL_GPUGraphicsPipeline* pipeline = CrearPipeline(gpu, window, vsh, fsh);
    
    // Los shaders ya no se necesitan tras crear el pipeline
    SDL_ReleaseGPUShader(gpu, vsh);
    SDL_ReleaseGPUShader(gpu, fsh);

    if (!pipeline) return 1;

    // 3. Creación y Subida del Buffer de Vértices (VBO)
    float vertices[] = {
         0.0f,  0.5f, 0.0f,  1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,
         0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f
    };

    SDL_GPUBuffer* vbo = SDL_CreateGPUBuffer(gpu, &(SDL_GPUBufferCreateInfo){
        .usage = SDL_GPU_BUFFERUSAGE_VERTEX, 
        .size = sizeof(vertices)
    });

    // Subida mediante Transfer Buffer (Staging)
    SDL_GPUTransferBuffer* upload = SDL_CreateGPUTransferBuffer(gpu, &(SDL_GPUTransferBufferCreateInfo){
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, 
        .size = sizeof(vertices)
    });
    void* ptr = SDL_MapGPUTransferBuffer(gpu, upload, false);
    SDL_memcpy(ptr, vertices, sizeof(vertices));
    SDL_UnmapGPUTransferBuffer(gpu, upload);

    SDL_GPUCommandBuffer* initCmd = SDL_AcquireGPUCommandBuffer(gpu);
    SDL_GPUCopyPass* copy = SDL_BeginGPUCopyPass(initCmd);
    SDL_UploadToGPUBuffer(copy, 
        &(SDL_GPUTransferBufferLocation){.transfer_buffer = upload}, 
        &(SDL_GPUBufferRegion){.buffer = vbo, .size = sizeof(vertices)}, false);
    SDL_EndGPUCopyPass(copy);
    SDL_SubmitGPUCommandBuffer(initCmd);
    SDL_ReleaseGPUTransferBuffer(gpu, upload);

    // 4. Bucle de Renderizado
    bool running = true;
    SDL_Event ev;
    while (running) {
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_EVENT_QUIT) running = false;
        }

        SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(gpu);
        if (!cmd) continue;

        SDL_GPUTexture* swapTex;
        if (SDL_AcquireGPUSwapchainTexture(cmd, window, &swapTex, NULL, NULL)) {
            if (swapTex != NULL) {
                SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(cmd, &(SDL_GPUColorTargetInfo){
                    .texture = swapTex, 
                    .clear_color = {0.02f, 0.02f, 0.02f, 1.0f},
                    .load_op = SDL_GPU_LOADOP_CLEAR, 
                    .store_op = SDL_GPU_STOREOP_STORE
                }, 1, NULL);

                SDL_BindGPUGraphicsPipeline(pass, pipeline);
                SDL_BindGPUVertexBuffers(pass, 0, &(SDL_GPUBufferBinding){ .buffer = vbo, .offset = 0 }, 1);

                // --- ACTUALIZACIÓN DE MOVIMIENTO ---
                Mat4 matriz;
                float tiempo = SDL_GetTicks() / 1000.0f; 
                MatrizRotacionZ(tiempo, &matriz);
                
                // Envío de datos al Shader
                SDL_PushGPUVertexUniformData(cmd, 0, &matriz, sizeof(Mat4));

                SDL_DrawGPUPrimitives(pass, 3, 1, 0, 0);
                SDL_EndGPURenderPass(pass);
            }
            SDL_SubmitGPUCommandBuffer(cmd);
        } else {
            SDL_CancelGPUCommandBuffer(cmd);
        }
    }

    // 5. Limpieza Final
    SDL_WaitForGPUIdle(gpu);
    SDL_ReleaseGPUBuffer(gpu, vbo);
    SDL_ReleaseGPUGraphicsPipeline(gpu, pipeline);
    SDL_ReleaseWindowFromGPUDevice(gpu, window);
    SDL_DestroyGPUDevice(gpu);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}