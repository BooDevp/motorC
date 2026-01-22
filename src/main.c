#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdbool.h>

typedef struct { float m[16]; } Mat4;

void MatrizRotacionZ(float angulo, Mat4* out) {
    float s = SDL_sinf(angulo);
    float c = SDL_cosf(angulo);
    // Inicializamos todo a 0
    for(int i=0; i<16; i++) out->m[i] = 0;
    
    // Matriz de rotación 2D (en el eje Z)
    out->m[0] = c;  out->m[1] = -s;
    out->m[4] = s;  out->m[5] = c;
    out->m[10] = 1; out->m[15] = 1;
}

static SDL_GPUShader* CargarShader(SDL_GPUDevice* gpu, const char* ruta, SDL_GPUShaderStage stage, uint32_t num_uniforms) {
    size_t size = 0;
    void* code = SDL_LoadFile(ruta, &size);
    if (!code) {
        SDL_Log("Error cargando shader: %s", ruta);
        return NULL;
    }
    SDL_GPUShaderCreateInfo info = {
        .code = code, 
        .code_size = size, 
        .entrypoint = "main",
        .format = SDL_GPU_SHADERFORMAT_SPIRV, 
        .stage = stage,
        .num_uniform_buffers = num_uniforms // <--- AQUÍ está el secreto
    };
    SDL_GPUShader* shader = SDL_CreateGPUShader(gpu, &info);
    SDL_free(code);
    return shader;
}

int main(int argc, char **argv) {
    if (!SDL_Init(SDL_INIT_VIDEO)) return 1;

    SDL_Window* window = SDL_CreateWindow("Motor SDL3 Estable", 800, 600, 0);
    // Modo debug true es fundamental para detectar por qué "no es estable"
    SDL_GPUDevice* gpu = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, NULL);
    if (!gpu || !window) return 1;

    SDL_ClaimWindowForGPUDevice(gpu, window);

    SDL_GPUShader* vsh = CargarShader(gpu, "src/shaders/simple.vert.spv", SDL_GPU_SHADERSTAGE_VERTEX,1);    
    SDL_GPUShader* fsh = CargarShader(gpu, "src/shaders/simple.frag.spv", SDL_GPU_SHADERSTAGE_FRAGMENT,0);
    if (!vsh || !fsh) return 1;

    SDL_GPUGraphicsPipelineCreateInfo pipeInfo = {
        .target_info = {
            .num_color_targets = 1,
            .color_target_descriptions = (SDL_GPUColorTargetDescription[]){{
                .format = SDL_GetGPUSwapchainTextureFormat(gpu, window),
                .blend_state = { // Añadimos estado de mezcla básico para estabilidad
                    .enable_blend = false,
                    .color_write_mask = 0xF
                }
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
                { .location = 0, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, .offset = 0 },
                { .location = 1, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, .offset = sizeof(float) * 3 }
            }
        }
    };

    SDL_GPUGraphicsPipeline* pipeline = SDL_CreateGPUGraphicsPipeline(gpu, &pipeInfo);
    SDL_ReleaseGPUShader(gpu, vsh);
    SDL_ReleaseGPUShader(gpu, fsh);
    if (!pipeline) return 1;

    float vertices[] = {
         0.0f,  0.5f, 0.0f,  1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,
         0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f
    };

    SDL_GPUBuffer* vbo = SDL_CreateGPUBuffer(gpu, &(SDL_GPUBufferCreateInfo){
        .usage = SDL_GPU_BUFFERUSAGE_VERTEX, .size = sizeof(vertices)
    });

    // Subida de datos (staging)
    SDL_GPUTransferBuffer* upload = SDL_CreateGPUTransferBuffer(gpu, &(SDL_GPUTransferBufferCreateInfo){
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, .size = sizeof(vertices)
    });
    void* ptr = SDL_MapGPUTransferBuffer(gpu, upload, false);
    SDL_memcpy(ptr, vertices, sizeof(vertices));
    SDL_UnmapGPUTransferBuffer(gpu, upload);

    SDL_GPUCommandBuffer* initCmd = SDL_AcquireGPUCommandBuffer(gpu);
    SDL_GPUCopyPass* copy = SDL_BeginGPUCopyPass(initCmd);
    SDL_UploadToGPUBuffer(copy, &(SDL_GPUTransferBufferLocation){.transfer_buffer = upload}, 
                         &(SDL_GPUBufferRegion){.buffer = vbo, .size = sizeof(vertices)}, false);
    SDL_EndGPUCopyPass(copy);
    SDL_SubmitGPUCommandBuffer(initCmd);
    SDL_ReleaseGPUTransferBuffer(gpu, upload);

    bool running = true;
    SDL_Event ev;
    while (running) {
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_EVENT_QUIT) running = false;
        }

        SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(gpu);
        if (!cmd) continue;

        SDL_GPUTexture* swapTex;
        uint32_t w, h;
        
        if (SDL_AcquireGPUSwapchainTexture(cmd, window, &swapTex, &w, &h)) {
            if (swapTex != NULL) {
                SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(cmd, &(SDL_GPUColorTargetInfo){
                    .texture = swapTex, 
                    .clear_color = {0.02f, 0.02f, 0.02f, 1.0f},
                    .load_op = SDL_GPU_LOADOP_CLEAR, 
                    .store_op = SDL_GPU_STOREOP_STORE
                }, 1, NULL);

                SDL_BindGPUGraphicsPipeline(pass, pipeline);
                SDL_BindGPUVertexBuffers(pass, 0, &(SDL_GPUBufferBinding){ .buffer = vbo, .offset = 0 }, 1);

                // --- BLOQUE DE MOVIMIENTO ---
                Mat4 matriz;
                float tiempo = SDL_GetTicks() / 1000.0f; 
                MatrizRotacionZ(tiempo, &matriz);
                
                // Enviamos la matriz al slot 0 del Vertex Shader
                SDL_PushGPUVertexUniformData(cmd, 0, &matriz, sizeof(Mat4));
                // ----------------------------

                SDL_DrawGPUPrimitives(pass, 3, 1, 0, 0);
                SDL_EndGPURenderPass(pass);
            }
            SDL_SubmitGPUCommandBuffer(cmd);
        } else {
            SDL_CancelGPUCommandBuffer(cmd);
        }
    }

    // Esperar a que la GPU termine antes de destruir (fundamental para estabilidad)
    SDL_WaitForGPUIdle(gpu);

    SDL_ReleaseGPUBuffer(gpu, vbo);
    SDL_ReleaseGPUGraphicsPipeline(gpu, pipeline);
    SDL_ReleaseWindowFromGPUDevice(gpu, window);
    SDL_DestroyGPUDevice(gpu);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}