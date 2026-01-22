#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdbool.h>
#include <math.h>

typedef struct { float m[16]; } Mat4;

// --- MATEMÁTICAS ---
void MultiplicarMat(Mat4* out, const Mat4* a, const Mat4* b) {
    Mat4 res = {0};
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 4; k++)
                res.m[i * 4 + j] += b->m[i * 4 + k] * a->m[k * 4 + j];
        }
    }
    *out = res;
}

void MatrizCubo3D(float tiempo, float aspecto, Mat4* out) {
    float s = SDL_sinf(tiempo);
    float c = SDL_cosf(tiempo);
    
    // 1. Inicializar todo a cero
    for(int i = 0; i < 16; i++) out->m[i] = 0.0f;

    // 2. Crear una matriz que solo rota en Z (como un ventilador de frente)
    // Esto nos asegura que el cubo no se aleja ni se acerca, solo gira.
    // Usamos el 'aspecto' para que no se vea estirado.
    out->m[0] = c / aspecto;  out->m[1] = -s;
    out->m[4] = s / aspecto;  out->m[5] = c;
    out->m[10] = 1.0f;
    out->m[15] = 1.0f; 
}

static SDL_GPUShader* CrearShader(SDL_GPUDevice* gpu, const char* ruta, SDL_GPUShaderStage stage, uint32_t num_uniforms) {
    size_t size = 0;
    void* code = SDL_LoadFile(ruta, &size);
    if (!code) return NULL;
    SDL_GPUShaderCreateInfo info = { .code = code, .code_size = size, .entrypoint = "main", .format = SDL_GPU_SHADERFORMAT_SPIRV, .stage = stage, .num_uniform_buffers = num_uniforms };
    SDL_GPUShader* shader = SDL_CreateGPUShader(gpu, &info);
    SDL_free(code);
    return shader;
}

int main(int argc, char **argv) {
    if (!SDL_Init(SDL_INIT_VIDEO)) return 1;

    // Usamos las mismas flags que tu triángulo estable
    SDL_Window* window = SDL_CreateWindow("Motor SDL3 - Cubo Seguro", 800, 600, SDL_WINDOW_RESIZABLE);
    SDL_GPUDevice* gpu = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, NULL);
    if (!window || !gpu) return 1;
    SDL_ClaimWindowForGPUDevice(gpu, window);

    SDL_GPUShader* vsh = CrearShader(gpu, "src/shaders/simple.vert.spv", SDL_GPU_SHADERSTAGE_VERTEX, 1);
    SDL_GPUShader* fsh = CrearShader(gpu, "src/shaders/simple.frag.spv", SDL_GPU_SHADERSTAGE_FRAGMENT, 0);

   SDL_GPUGraphicsPipeline* pipeline = SDL_CreateGPUGraphicsPipeline(gpu, &(SDL_GPUGraphicsPipelineCreateInfo){
        .target_info = {
            .num_color_targets = 1,
            .color_target_descriptions = (SDL_GPUColorTargetDescription[]){{.format = SDL_GetGPUSwapchainTextureFormat(gpu, window)}},
            .has_depth_stencil_target = true,
            .depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D16_UNORM
        },
        .depth_stencil_state = { 
            .enable_depth_test = true, 
            .enable_depth_write = true, 
            .compare_op = SDL_GPU_COMPAREOP_LESS 
        },
        // AQUÍ VA EL RASTERIZER STATE:
        .rasterizer_state = {
            .cull_mode = SDL_GPU_CULLMODE_BACK,    // No dibuja las caras de atrás
            .front_face = SDL_GPU_FRONTFACE_CLOCKWISE, // Define qué cara es la de "delante"
            .fill_mode = SDL_GPU_FILLMODE_FILL     // Rellena los triángulos
        },
        .vertex_shader = vsh, 
        .fragment_shader = fsh,
        .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
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
    SDL_ReleaseGPUShader(gpu, vsh); SDL_ReleaseGPUShader(gpu, fsh);

    float vertices[] = {
        -0.5f,-0.5f, 0.5f, 1,0,0,  0.5f,-0.5f, 0.5f, 1,0,0,  0.5f, 0.5f, 0.5f, 1,0,0, -0.5f,-0.5f, 0.5f, 1,0,0,  0.5f, 0.5f, 0.5f, 1,0,0, -0.5f, 0.5f, 0.5f, 1,0,0,
        -0.5f,-0.5f,-0.5f, 0,1,0,  0.5f, 0.5f,-0.5f, 0,1,0,  0.5f,-0.5f,-0.5f, 0,1,0, -0.5f,-0.5f,-0.5f, 0,1,0, -0.5f, 0.5f,-0.5f, 0,1,0,  0.5f, 0.5f,-0.5f, 0,1,0,
        -0.5f, 0.5f, 0.5f, 0,0,1,  0.5f, 0.5f, 0.5f, 0,0,1,  0.5f, 0.5f,-0.5f, 0,0,1, -0.5f, 0.5f, 0.5f, 0,0,1,  0.5f, 0.5f,-0.5f, 0,0,1, -0.5f, 0.5f,-0.5f, 0,0,1,
        -0.5f,-0.5f, 0.5f, 1,1,0,  0.5f,-0.5f,-0.5f, 1,1,0,  0.5f,-0.5f, 0.5f, 1,1,0, -0.5f,-0.5f, 0.5f, 1,1,0, -0.5f,-0.5f,-0.5f, 1,1,0,  0.5f,-0.5f,-0.5f, 1,1,0,
         0.5f,-0.5f, 0.5f, 0,1,1,  0.5f,-0.5f,-0.5f, 0,1,1,  0.5f, 0.5f,-0.5f, 0,1,1,  0.5f,-0.5f, 0.5f, 0,1,1,  0.5f, 0.5f,-0.5f, 0,1,1,  0.5f, 0.5f, 0.5f, 0,1,1,
        -0.5f,-0.5f, 0.5f, 1,0,1, -0.5f, 0.5f,-0.5f, 1,0,1, -0.5f,-0.5f,-0.5f, 1,0,1, -0.5f,-0.5f, 0.5f, 1,0,1, -0.5f, 0.5f, 0.5f, 1,0,1, -0.5f, 0.5f,-0.5f, 1,0,1
    };

    SDL_GPUBuffer* vbo = SDL_CreateGPUBuffer(gpu, &(SDL_GPUBufferCreateInfo){.usage = SDL_GPU_BUFFERUSAGE_VERTEX, .size = sizeof(vertices)});
    SDL_GPUTransferBuffer* up = SDL_CreateGPUTransferBuffer(gpu, &(SDL_GPUTransferBufferCreateInfo){.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, .size = sizeof(vertices)});
    void* p = SDL_MapGPUTransferBuffer(gpu, up, false); SDL_memcpy(p, vertices, sizeof(vertices)); SDL_UnmapGPUTransferBuffer(gpu, up);
    SDL_GPUCommandBuffer* ic = SDL_AcquireGPUCommandBuffer(gpu);
    SDL_GPUCopyPass* cp = SDL_BeginGPUCopyPass(ic); SDL_UploadToGPUBuffer(cp, &(SDL_GPUTransferBufferLocation){.transfer_buffer = up}, &(SDL_GPUBufferRegion){.buffer = vbo, .size = sizeof(vertices)}, false);
    SDL_EndGPUCopyPass(cp); SDL_SubmitGPUCommandBuffer(ic); SDL_ReleaseGPUTransferBuffer(gpu, up);

    SDL_GPUTexture* depth = NULL;
    uint32_t dw = 0, dh = 0;

    bool running = true;
    while (running) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_EVENT_QUIT) running = false;
        }

        int w, h;
        SDL_GetWindowSizeInPixels(window, &w, &h);
        if (w <= 0 || h <= 0) continue;

        // GESTIÓN DEL DEPTH BUFFER
        if (depth == NULL || (uint32_t)w != dw || (uint32_t)h != dh) {
            SDL_WaitForGPUIdle(gpu); // PARADA TOTAL para evitar crash
            if (depth) SDL_ReleaseGPUTexture(gpu, depth);
            dw = w; dh = h;
            depth = SDL_CreateGPUTexture(gpu, &(SDL_GPUTextureCreateInfo){
                .type = SDL_GPU_TEXTURETYPE_2D, .format = SDL_GPU_TEXTUREFORMAT_D16_UNORM,
                .width = dw, .height = dh, .layer_count_or_depth = 1, .num_levels = 1,
                .usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET
            });
            if (!depth) continue;
        }

        SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(gpu);
        if (!cmd) continue;

        SDL_GPUTexture* swap;
        if (SDL_AcquireGPUSwapchainTexture(cmd, window, &swap, NULL, NULL)) {
            if (swap) {
                SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(cmd, 
                    &(SDL_GPUColorTargetInfo){ .texture = swap, .clear_color = {0,0,0,1}, .load_op = SDL_GPU_LOADOP_CLEAR, .store_op = SDL_GPU_STOREOP_STORE }, 1,
                    &(SDL_GPUDepthStencilTargetInfo){ .texture = depth, .clear_depth = 1.0f, .load_op = SDL_GPU_LOADOP_CLEAR, .store_op = SDL_GPU_STOREOP_DONT_CARE }
                );

                SDL_BindGPUGraphicsPipeline(pass, pipeline);
                SDL_BindGPUVertexBuffers(pass, 0, &(SDL_GPUBufferBinding){.buffer = vbo}, 1);
                Mat4 mvp; MatrizCubo3D(SDL_GetTicks()/1000.0f, (float)w/h, &mvp);
                SDL_PushGPUVertexUniformData(cmd, 0, &mvp, sizeof(Mat4));
                SDL_DrawGPUPrimitives(pass, 36, 1, 0, 0);
                SDL_EndGPURenderPass(pass);
            }
            SDL_SubmitGPUCommandBuffer(cmd);
        } else {
            SDL_CancelGPUCommandBuffer(cmd);
        }
    }

    SDL_WaitForGPUIdle(gpu);
    if (depth) SDL_ReleaseGPUTexture(gpu, depth);
    SDL_ReleaseGPUBuffer(gpu, vbo);
    SDL_ReleaseGPUGraphicsPipeline(gpu, pipeline);
    SDL_DestroyGPUDevice(gpu);
    SDL_Quit();
    return 0;
}