#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

// --- FUNCIONES DE APOYO ---

// Carga el código binario SPIR-V y crea el objeto de shader en la GPU
SDL_GPUShader* CargarShader(SDL_GPUDevice* gpu, const char* ruta, SDL_GPUShaderStage stage) {
    size_t tam;
    void* codigo = SDL_LoadFile(ruta, &tam);
    if (!codigo) {
        SDL_Log("No se pudo leer el archivo: %s", ruta);
        return NULL;
    }

    SDL_GPUShaderCreateInfo info = {
        .code = codigo,
        .code_size = tam,
        .entrypoint = "main",
        .format = SDL_GPU_SHADERFORMAT_SPIRV,
        .stage = stage
    };

    SDL_GPUShader* shader = SDL_CreateGPUShader(gpu, &info);
    SDL_free(codigo);
    return shader;
}

// Crea un buffer de vértices y sube los datos a la VRAM (memoria de video)
SDL_GPUBuffer* CrearBufferVertices(SDL_GPUDevice* gpu, float* vertices, size_t tam) {
    // 1. Reservar espacio en la GPU
    SDL_GPUBuffer* buffer = SDL_CreateGPUBuffer(gpu, &(SDL_GPUBufferCreateInfo){
        .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
        .size = tam
    });

    // 2. Usar un transfer buffer para la mudanza de datos
    SDL_GPUTransferBuffer* staging = SDL_CreateGPUTransferBuffer(gpu, &(SDL_GPUTransferBufferCreateInfo){
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        .size = tam
    });

    float* dataPtr = (float*)SDL_MapGPUTransferBuffer(gpu, staging, false);
    SDL_memcpy(dataPtr, vertices, tam);
    SDL_UnmapGPUTransferBuffer(gpu, staging);

    // 3. Ejecutar la copia inmediata
    SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(gpu);
    SDL_GPUCopyPass* copy = SDL_BeginGPUCopyPass(cmd);
    SDL_UploadToGPUBuffer(copy, 
        &(SDL_GPUTransferBufferLocation){.transfer_buffer = staging},
        &(SDL_GPUBufferRegion){.buffer = buffer, .size = tam}, 
        false);
    SDL_EndGPUCopyPass(copy);
    SDL_SubmitGPUCommandBuffer(cmd);

    SDL_ReleaseGPUTransferBuffer(gpu, staging);
    return buffer;
}

// Configura la "receta" de cómo se deben dibujar los datos
SDL_GPUGraphicsPipeline* CrearPipeline(SDL_GPUDevice* gpu, SDL_Window* window) {
    SDL_GPUShader* vsh = CargarShader(gpu, "src/shaders/simple.vert.spv", SDL_GPU_SHADERSTAGE_VERTEX);
    SDL_GPUShader* fsh = CargarShader(gpu, "src/shaders/simple.frag.spv", SDL_GPU_SHADERSTAGE_FRAGMENT);

    if (!vsh || !fsh) return NULL;

    SDL_GPUGraphicsPipelineCreateInfo info = {
        .target_info = {
            .num_color_targets = 1,
            .color_target_descriptions = (SDL_GPUColorTargetDescription[]){{
                .format = SDL_GetGPUSwapchainTextureFormat(gpu, window)
            }}
        },
        .vertex_shader = vsh,
        .fragment_shader = fsh,
        .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
        .vertex_input_state = {
            .num_vertex_buffers = 1,
            .vertex_buffer_descriptions = (SDL_GPUVertexBufferDescription[]){{.slot = 0, .pitch = sizeof(float) * 3}},
            .num_vertex_attributes = 1,
            .vertex_attributes = (SDL_GPUVertexAttribute[]){{.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, .location = 0}}
        }
    };

    SDL_GPUGraphicsPipeline* pipeline = SDL_CreateGPUGraphicsPipeline(gpu, &info);
    
    SDL_ReleaseGPUShader(gpu, vsh);
    SDL_ReleaseGPUShader(gpu, fsh);
    
    return pipeline;
}

// --- MAIN PRINCIPAL ---

int main(int argc, char *argv[]) {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow("Motor v2 - Triangulo Limpio", 800, 600, 0);
    SDL_GPUDevice* gpu = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, false, NULL);
    SDL_ClaimWindowForGPUDevice(gpu, window);

    // DATOS: Definimos y subimos
    float vertices[] = { 0.0f, 0.5f, 0.0f, -0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f };
    SDL_GPUBuffer* vertexBuffer = CrearBufferVertices(gpu, vertices, sizeof(vertices));
    
    // RECETA: Creamos el pipeline
    SDL_GPUGraphicsPipeline* pipeline = CrearPipeline(gpu, window);

    bool corriendo = true;
    SDL_Event ev;

    while (corriendo) {
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_EVENT_QUIT) corriendo = false;
        }

        SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(gpu);
        if (cmd) {
            SDL_GPUTexture* swapchainTex;
            if (SDL_AcquireGPUSwapchainTexture(cmd, window, &swapchainTex, NULL, NULL)) {
                
                SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(cmd, &(SDL_GPUColorTargetInfo){
                    .texture = swapchainTex,
                    .clear_color = { 0.1f, 0.15f, 0.2f, 1.0f },
                    .load_op = SDL_GPU_LOADOP_CLEAR,
                    .store_op = SDL_GPU_STOREOP_STORE
                }, 1, NULL);

                SDL_BindGPUGraphicsPipeline(pass, pipeline);
                SDL_BindGPUVertexBuffers(pass, 0, &(SDL_GPUBufferBinding){.buffer = vertexBuffer}, 1);
                SDL_DrawGPUPrimitives(pass, 3, 1, 0, 0);

                SDL_EndGPURenderPass(pass);
            }
            SDL_SubmitGPUCommandBuffer(cmd);
        }
        SDL_Delay(16);
    }

    // LIMPIEZA
    SDL_ReleaseGPUGraphicsPipeline(gpu, pipeline);
    SDL_ReleaseGPUBuffer(gpu, vertexBuffer);
    SDL_ReleaseWindowFromGPUDevice(gpu, window);
    SDL_DestroyGPUDevice(gpu);
    SDL_Quit();

    return 0;
}