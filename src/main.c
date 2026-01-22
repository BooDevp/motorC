#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

SDL_GPUShader* CargarShader(SDL_GPUDevice* gpu, const char* ruta, SDL_GPUShaderStage stage) {
    size_t tam;
    void* codigo = SDL_LoadFile(ruta, &tam);
    if (!codigo) return NULL;

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

int main(int argc, char *argv[]) {
    // 1. Inicializar solo Video
    if (!SDL_Init(SDL_INIT_VIDEO)) return 1;

    // 2. Crear ventana
    SDL_Window *window = SDL_CreateWindow("Paso 1: Color Simple", 800, 600, 0);
    if (!window) return 1;

    // 3. Crear el dispositivo GPU (Vulkan por defecto en NVIDIA)
    SDL_GPUDevice *gpu = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, false, NULL);
    if (!gpu) return 1;

    // 4. Vincular la ventana a la GPU
    SDL_ClaimWindowForGPUDevice(gpu, window);

    bool ejecutando = true;
    SDL_Event evento;

    SDL_GPUCommandBuffer *copyCmd = SDL_AcquireGPUCommandBuffer(gpu);
    SDL_GPUCopyPass *copyPass = SDL_BeginGPUCopyPass(copyCmd);

    float vertices[] = {
         0.0f,  0.5f, 0.0f,  // Arriba
        -0.5f, -0.5f, 0.0f,  // Izquierda
         0.5f, -0.5f, 0.0f   // Derecha
    };

    // 1. Crear la descripción del buffer (¿Para qué es y cuánto mide?)
    SDL_GPUBufferCreateInfo buffer_info = {
        .usage = SDL_GPU_BUFFERUSAGE_VERTEX, // Es para vértices
        .size = sizeof(vertices)             // Tamaño: 9 floats * 4 bytes = 36 bytes
    };

    // 2. Pedirle a la GPU que reserve ese espacio
    SDL_GPUBuffer *vertexBuffer = SDL_CreateGPUBuffer(gpu, &buffer_info);

    // 3. Crear un "Transfer Buffer" (El camión de mudanza)
    // La GPU no nos deja escribir directo en vertexBuffer, 
    // así que usamos un buffer intermedio para subir los datos.
    SDL_GPUTransferBufferCreateInfo transfer_info = {
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        .size = sizeof(vertices)
    };
    SDL_GPUTransferBuffer *stagingBuffer = SDL_CreateGPUTransferBuffer(gpu, &transfer_info);

    // 4. Copiar los datos de la CPU al "camión" (stagingBuffer)
    float *dataPtr = (float*)SDL_MapGPUTransferBuffer(gpu, stagingBuffer, false);
    SDL_memcpy(dataPtr, vertices, sizeof(vertices));
    SDL_UnmapGPUTransferBuffer(gpu, stagingBuffer);

    SDL_UploadToGPUBuffer(
        copyPass,
        &(SDL_GPUTransferBufferLocation) { .transfer_buffer = stagingBuffer, .offset = 0 },
        &(SDL_GPUBufferRegion) { .buffer = vertexBuffer, .size = sizeof(vertices), .offset = 0 },
        false
    );

    SDL_EndGPUCopyPass(copyPass);
    SDL_SubmitGPUCommandBuffer(copyCmd);

    // Ya no necesitamos el "camión", los datos ya están en el almacén de la GPU
    SDL_ReleaseGPUTransferBuffer(gpu, stagingBuffer);

    // 1. Cargamos los shaders compilados (asegúrate de haberlos compilado a .spv)
    SDL_GPUShader* vertexShader = CargarShader(gpu, "src/shaders/simple.vert.spv", SDL_GPU_SHADERSTAGE_VERTEX);
    SDL_GPUShader* fragmentShader = CargarShader(gpu, "src/shaders/simple.frag.spv", SDL_GPU_SHADERSTAGE_FRAGMENT);

    // Comprobación de seguridad (para que no se cierre sin avisar)
    if (!vertexShader || !fragmentShader) {
        SDL_Log("¡ERROR! No se encontraron los archivos .spv en src/shaders/");
        return 1;
    }

    // 2. Creamos la "Receta" completa
    SDL_GPUGraphicsPipelineCreateInfo pipInfo = {
        .target_info = {
            .num_color_targets = 1,
            .color_target_descriptions = (SDL_GPUColorTargetDescription[]){{
                .format = SDL_GetGPUSwapchainTextureFormat(gpu, window)
            }}
        },
        .vertex_shader = vertexShader,
        .fragment_shader = fragmentShader,
        .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST, // ¡Aquí decimos que es un triángulo!
        .vertex_input_state = {
            .num_vertex_buffers = 1, // Cambiado de num_vertex_buffer_descriptions
            .vertex_buffer_descriptions = (SDL_GPUVertexBufferDescription[]){{
                .slot = 0,
                .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
                .instance_step_rate = 0,
                .pitch = sizeof(float) * 3
            }},
            .num_vertex_attributes = 1,
            .vertex_attributes = (SDL_GPUVertexAttribute[]){{
                .buffer_slot = 0,
                .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
                .location = 0,
                .offset = 0
            }}
        }
    };

    SDL_GPUGraphicsPipeline* pipeline = SDL_CreateGPUGraphicsPipeline(gpu, &pipInfo);
    if (!pipeline) {
        SDL_Log("¡ERROR! No se pudo crear el Pipeline. Revisa los shaders.");
        return 1;
    }

    // Ya podemos liberar los shaders individuales, el pipeline ya los tiene dentro
    SDL_ReleaseGPUShader(gpu, vertexShader);
    SDL_ReleaseGPUShader(gpu, fragmentShader);

    while (ejecutando) {
        while (SDL_PollEvent(&evento)) {
            if (evento.type == SDL_EVENT_QUIT) ejecutando = false;
        }

        // --- RENDERIZADO ---
       SDL_GPUCommandBuffer *cmd = SDL_AcquireGPUCommandBuffer(gpu);
    if (cmd) {
        SDL_GPUTexture *textura_swapchain;
        if (SDL_AcquireGPUSwapchainTexture(cmd, window, &textura_swapchain, NULL, NULL)) {
            
            SDL_GPUColorTargetInfo color_info = {
                .texture = textura_swapchain,
                .clear_color = { 0.1f, 0.15f, 0.2f, 1.0f }, 
                .load_op = SDL_GPU_LOADOP_CLEAR,
                .store_op = SDL_GPU_STOREOP_STORE
            };

            SDL_GPURenderPass *pass = SDL_BeginGPURenderPass(cmd, &color_info, 1, NULL);
            
            // --- AQUÍ OCURRE LA MAGIA ---
            // 1. Decimos qué receta usar
            SDL_BindGPUGraphicsPipeline(pass, pipeline);
            
            // 2. Decimos qué datos usar (el buffer que llenamos al principio)
            SDL_BindGPUVertexBuffers(pass, 0, &(SDL_GPUBufferBinding){ .buffer = vertexBuffer, .offset = 0 }, 1);
            
            // 3. ¡Dibuja! (3 vértices, 1 instancia)
            SDL_DrawGPUPrimitives(pass, 3, 1, 0, 0);
            // ----------------------------

            SDL_EndGPURenderPass(pass);
        }
        SDL_SubmitGPUCommandBuffer(cmd);
    }

        SDL_Delay(16); // Esto limita el programa a unos 60 FPS
    }

    // Limpieza
    SDL_ReleaseWindowFromGPUDevice(gpu, window);
    SDL_DestroyGPUDevice(gpu);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}