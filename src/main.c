#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

typedef struct {
    float m[16];
} Mat4;

int main(int argc, char **argv) {
    SDL_Log("SISTEMA: Iniciando SDL...");
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("ERROR SDL: %s", SDL_GetError());
        return 1;
    }

    SDL_Window *win = SDL_CreateWindow("Debugger Cuadrado", 800, 600, 0);
    if (!win) {
        SDL_Log("ERROR Ventana: %s", SDL_GetError());
        return 1;
    }

    SDL_Log("SISTEMA: Creando dispositivo GPU...");
    SDL_GPUDevice *gpu = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, NULL);
    if (!gpu) {
        SDL_Log("ERROR GPU: No se pudo crear el dispositivo.");
        return 1;
    }
    SDL_ClaimWindowForGPUDevice(gpu, win);

    // 1. CARGA DE SHADERS
    size_t vsz, fsz;
    void *vcode = SDL_LoadFile("src/shaders/simple.vert.spv", &vsz);
    void *fcode = SDL_LoadFile("src/shaders/simple.frag.spv", &fsz);
    if (!vcode || !fcode) {
        SDL_Log("ERROR: No se encuentran los archivos .spv");
        return 1;
    }

    SDL_GPUShader *vsh = SDL_CreateGPUShader(gpu, &(SDL_GPUShaderCreateInfo){
        .code = vcode, .code_size = vsz, .entrypoint = "main", 
        .format = SDL_GPU_SHADERFORMAT_SPIRV, .stage = SDL_GPU_SHADERSTAGE_VERTEX, .num_uniform_buffers = 1
    });

    SDL_GPUShader *fsh = SDL_CreateGPUShader(gpu, &(SDL_GPUShaderCreateInfo){
        .code = fcode, .code_size = fsz, .entrypoint = "main", 
        .format = SDL_GPU_SHADERFORMAT_SPIRV, .stage = SDL_GPU_SHADERSTAGE_FRAGMENT
    });

    // 2. CONFIGURACIÓN DEL PIPELINE
    SDL_GPUGraphicsPipeline *pip = SDL_CreateGPUGraphicsPipeline(gpu, &(SDL_GPUGraphicsPipelineCreateInfo){
        .target_info = {
            .num_color_targets = 1,
            .color_target_descriptions = (SDL_GPUColorTargetDescription[]){{.format = SDL_GetGPUSwapchainTextureFormat(gpu, win)}}
        },
        .vertex_shader = vsh,
        .fragment_shader = fsh,
        .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
        // Evitamos que NVIDIA descarte caras "traseras"
        .rasterizer_state = {.cull_mode = SDL_GPU_CULLMODE_NONE, .fill_mode = SDL_GPU_FILLMODE_FILL},
        .vertex_input_state = {
            .num_vertex_buffers = 1,
            .vertex_buffer_descriptions = (SDL_GPUVertexBufferDescription[]){{.slot = 0, .pitch = sizeof(float) * 6}},
            .num_vertex_attributes = 2,
            .vertex_attributes = (SDL_GPUVertexAttribute[]){
                {.location = 0, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, .offset = 0},
                {.location = 1, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, .offset = sizeof(float) * 3}
            }
        }
    });

    // 3. DATOS DE VÉRTICES Y MATRIZ
    float verts[] = {
        -0.5f, -0.5f, 0.0f, 1, 0, 0, // Rojo
         0.5f, -0.5f, 0.0f, 0, 1, 0, // Verde
         0.5f,  0.5f, 0.0f, 0, 0, 1, // Azul
        -0.5f, -0.5f, 0.0f, 1, 0, 0, // Rojo
         0.5f,  0.5f, 0.0f, 0, 0, 1, // Azul
        -0.5f,  0.5f, 0.0f, 1, 1, 1  // Blanco
    };

    Mat4 identidad = {.m = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1}};

    // 4. CREACIÓN Y SUBIDA A BUFFERS
    SDL_GPUBuffer *vbo = SDL_CreateGPUBuffer(gpu, &(SDL_GPUBufferCreateInfo){.usage = SDL_GPU_BUFFERUSAGE_VERTEX, .size = sizeof(verts)});
    SDL_GPUTransferBuffer *tb = SDL_CreateGPUTransferBuffer(gpu, &(SDL_GPUTransferBufferCreateInfo){.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, .size = sizeof(verts)});

    void *map = SDL_MapGPUTransferBuffer(gpu, tb, false);
    SDL_memcpy(map, verts, sizeof(verts));
    SDL_UnmapGPUTransferBuffer(gpu, tb);

    SDL_GPUCommandBuffer *setup = SDL_AcquireGPUCommandBuffer(gpu);
    SDL_GPUCopyPass *copy = SDL_BeginGPUCopyPass(setup);
    SDL_UploadToGPUBuffer(copy, &(SDL_GPUTransferBufferLocation){.transfer_buffer = tb}, &(SDL_GPUBufferRegion){.buffer = vbo, .size = sizeof(verts)}, false);
    SDL_EndGPUCopyPass(copy);
    SDL_SubmitGPUCommandBuffer(setup);

    // 5. BUCLE PRINCIPAL
    SDL_Log(">>> ENTRANDO EN BUCLE PRINCIPAL <<<");
    bool running = true;
    while (running) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_EVENT_QUIT) running = false;
        }

        SDL_GPUCommandBuffer *drawCmd = SDL_AcquireGPUCommandBuffer(gpu);
        if (!drawCmd) continue;

        SDL_GPUTexture *swap;
        if (SDL_AcquireGPUSwapchainTexture(drawCmd, win, &swap, NULL, NULL)) {
            SDL_GPURenderPass *pass = SDL_BeginGPURenderPass(drawCmd, &(SDL_GPUColorTargetInfo){
                .texture = swap, .clear_color = {0.1f, 0.1f, 0.1f, 1.0f}, .load_op = SDL_GPU_LOADOP_CLEAR
            }, 1, NULL);

            SDL_BindGPUGraphicsPipeline(pass, pip);
            SDL_BindGPUVertexBuffers(pass, 0, &(SDL_GPUBufferBinding){.buffer = vbo}, 1);
            
            // Enviamos la matriz a la GPU
            SDL_PushGPUVertexUniformData(drawCmd, 0, &identidad, sizeof(Mat4));
            
            SDL_DrawGPUPrimitives(pass, 6, 1, 0, 0);
            SDL_EndGPURenderPass(pass);
            SDL_SubmitGPUCommandBuffer(drawCmd);
        } else {
            SDL_CancelGPUCommandBuffer(drawCmd);
        }
        SDL_Delay(16);
    }

    SDL_Log("SISTEMA: Limpiando y saliendo...");
    SDL_ReleaseGPUTransferBuffer(gpu, tb);
    SDL_ReleaseGPUBuffer(gpu, vbo);
    SDL_DestroyGPUDevice(gpu);
    SDL_Quit();
    return 0;
}