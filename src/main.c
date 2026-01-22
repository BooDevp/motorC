#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdbool.h>
#include <math.h>

// --- AJUSTES RÁPIDOS ---
#define WINDOW_W 800
#define WINDOW_H 600
#define FOV_GRADOS 60.0f
#define CAM_DIST   3.0f
#define ROT_VEL    1.0f

typedef struct { float m[16]; } Mat4;

// --- MATEMÁTICAS ---

void MatIdentidad(Mat4* m) {
    for (int i = 0; i < 16; i++) m->m[i] = 0.0f;
    m->m[0] = m->m[5] = m->m[10] = m->m[15] = 1.0f;
}

void MultiplicarMat(Mat4* out, const Mat4* a, const Mat4* b) {
    Mat4 res = {0};
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 4; k++) {
                res.m[i * 4 + j] += a->m[i * 4 + k] * b->m[k * 4 + j];
            }
        }
    }
    *out = res;
}

void MatrizCubo3D(float tiempo, float aspecto, Mat4* out) {
    float t = tiempo * ROT_VEL;
    float s = SDL_sinf(t);
    float c = SDL_cosf(t);

    // 1. ROTACIÓN Y (Giro de peonza)
    Mat4 rotY; MatIdentidad(&rotY);
    rotY.m[0] = c;  rotY.m[2] = s;
    rotY.m[8] = -s; rotY.m[10] = c;

    // 2. TRASLACIÓN Z (Mover el objeto al fondo)
    // El valor tx, ty, tz va en la última columna (índices 3, 7, 11)
    Mat4 trans; MatIdentidad(&trans);
    trans.m[11] = CAM_DIST; 

    // 3. PROYECCIÓN PERSPECTIVA (Vulkan/SDL3 Style)
    float near = 0.1f;
    float far = 100.0f;
    float f = 1.0f / SDL_tanf(FOV_GRADOS * 0.5f * (3.14159f / 180.0f));
    
    Mat4 proj = {0};
    proj.m[0] = f / aspecto;
    proj.m[5] = f;
    proj.m[10] = far / (far - near);           // Escala Z
    proj.m[11] = (-far * near) / (far - near); // Traslación Z para Depth
    proj.m[14] = 1.0f;                         // ¡ESTO GENERA EL W! (W = Z)

    // 4. COMBINAR: Proyección * Traslación * Rotación
    Mat4 tmp;
    MultiplicarMat(&tmp, &trans, &rotY); 
    MultiplicarMat(out, &proj, &tmp);
}

// --- SHADERS ---
// (Tu función CrearShader está perfecta, no la toques)
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

    SDL_Window* window = SDL_CreateWindow("Cubo Corregido", WINDOW_W, WINDOW_H, SDL_WINDOW_RESIZABLE);
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
        .depth_stencil_state = { .enable_depth_test = true, .enable_depth_write = true, .compare_op = SDL_GPU_COMPAREOP_LESS },
        .rasterizer_state = { .cull_mode = SDL_GPU_CULLMODE_BACK, .fill_mode = SDL_GPU_FILLMODE_FILL },
        .vertex_shader = vsh, .fragment_shader = fsh,
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
        // Cara Z+ (Rojo)
        -1,-1, 1, 1,0,0,  1,-1, 1, 1,0,0,  1, 1, 1, 1,0,0, -1,-1, 1, 1,0,0,  1, 1, 1, 1,0,0, -1, 1, 1, 1,0,0,
        // Cara Z- (Verde)
        -1,-1,-1, 0,1,0,  1, 1,-1, 0,1,0,  1,-1,-1, 0,1,0, -1,-1,-1, 0,1,0, -1, 1,-1, 0,1,0,  1, 1,-1, 0,1,0,
        // Cara Y+ (Azul)
        -1, 1, 1, 0,0,1,  1, 1, 1, 0,0,1,  1, 1,-1, 0,0,1, -1, 1, 1, 0,0,1,  1, 1,-1, 0,0,1, -1, 1,-1, 0,0,1,
        // Cara Y- (Amarillo)
        -1,-1, 1, 1,1,0,  1,-1,-1, 1,1,0,  1,-1, 1, 1,1,0, -1,-1, 1, 1,1,0, -1,-1,-1, 1,1,0,  1,-1,-1, 1,1,0,
        // Cara X+ (Cian)
         1,-1, 1, 0,1,1,  1,-1,-1, 0,1,1,  1, 1,-1, 0,1,1,  1,-1, 1, 0,1,1,  1, 1,-1, 0,1,1,  1, 1, 1, 0,1,1,
        // Cara X- (Magenta)
        -1,-1, 1, 1,0,1, -1, 1,-1, 1,0,1, -1,-1,-1, 1,0,1, -1,-1, 1, 1,0,1, -1, 1, 1, 1,0,1, -1, 1,-1, 1,0,1
    };

    SDL_GPUBuffer* vbo = SDL_CreateGPUBuffer(gpu, &(SDL_GPUBufferCreateInfo){.usage = SDL_GPU_BUFFERUSAGE_VERTEX, .size = sizeof(vertices)});
    SDL_GPUTransferBuffer* up = SDL_CreateGPUTransferBuffer(gpu, &(SDL_GPUTransferBufferCreateInfo){.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, .size = sizeof(vertices)});
    void* ptr = SDL_MapGPUTransferBuffer(gpu, up, false); SDL_memcpy(ptr, vertices, sizeof(vertices)); SDL_UnmapGPUTransferBuffer(gpu, up);
    SDL_GPUCommandBuffer* ic = SDL_AcquireGPUCommandBuffer(gpu);
    SDL_GPUCopyPass* cp = SDL_BeginGPUCopyPass(ic); SDL_UploadToGPUBuffer(cp, &(SDL_GPUTransferBufferLocation){.transfer_buffer = up}, &(SDL_GPUBufferRegion){.buffer = vbo, .size = sizeof(vertices)}, false);
    SDL_EndGPUCopyPass(cp); SDL_SubmitGPUCommandBuffer(ic); SDL_ReleaseGPUTransferBuffer(gpu, up);

    SDL_GPUTexture* depth = NULL;
    uint32_t dw = 0, dh = 0;
    bool running = true;

    while (running) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) if (ev.type == SDL_EVENT_QUIT) running = false;

        int w, h;
        SDL_GetWindowSizeInPixels(window, &w, &h);
        if (w <= 0 || h <= 0) continue;

        if (depth == NULL || (uint32_t)w != dw || (uint32_t)h != dh) {
            SDL_WaitForGPUIdle(gpu);
            if (depth) SDL_ReleaseGPUTexture(gpu, depth);
            dw = (uint32_t)w; dh = (uint32_t)h;
            depth = SDL_CreateGPUTexture(gpu, &(SDL_GPUTextureCreateInfo){ .type = SDL_GPU_TEXTURETYPE_2D, .format = SDL_GPU_TEXTUREFORMAT_D16_UNORM, .width = dw, .height = dh, .layer_count_or_depth = 1, .num_levels = 1, .usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET });
        }

        SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(gpu);
        SDL_GPUTexture* swap;
        if (SDL_AcquireGPUSwapchainTexture(cmd, window, &swap, NULL, NULL) && swap) {
            SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(cmd, 
                &(SDL_GPUColorTargetInfo){ .texture = swap, .clear_color = {0.1f, 0.1f, 0.1f, 1.0f}, .load_op = SDL_GPU_LOADOP_CLEAR, .store_op = SDL_GPU_STOREOP_STORE }, 1,
                &(SDL_GPUDepthStencilTargetInfo){ .texture = depth, .clear_depth = 1.0f, .load_op = SDL_GPU_LOADOP_CLEAR, .store_op = SDL_GPU_STOREOP_DONT_CARE }
            );

            SDL_BindGPUGraphicsPipeline(pass, pipeline);
            SDL_BindGPUVertexBuffers(pass, 0, &(SDL_GPUBufferBinding){.buffer = vbo}, 1);
            
            Mat4 mvp; 
            MatrizCubo3D((float)SDL_GetTicks()/1000.0f, (float)w/h, &mvp);
            
            SDL_PushGPUVertexUniformData(cmd, 0, &mvp, sizeof(Mat4));
            SDL_DrawGPUPrimitives(pass, 36, 1, 0, 0);
            SDL_EndGPURenderPass(pass);
            SDL_SubmitGPUCommandBuffer(cmd);
        } else {
            SDL_CancelGPUCommandBuffer(cmd);
        }
    }

    SDL_WaitForGPUIdle(gpu);
    SDL_ReleaseGPUTexture(gpu, depth);
    SDL_ReleaseGPUBuffer(gpu, vbo);
    SDL_ReleaseGPUGraphicsPipeline(gpu, pipeline);
    SDL_DestroyGPUDevice(gpu);
    SDL_Quit();
    return 0;
}