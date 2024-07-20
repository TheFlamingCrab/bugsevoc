#include <webgpu/wgpu.h>
#include <SDL2/SDL.h>
#include <sdl2webgpu.h>

typedef struct {
    float position[2];
    float size;
} InstanceRaw;

InstanceRaw instances[10];

struct State {
    WGPUDevice device;
    WGPUQueue queue;
    SDL_Window* window;
    WGPUSurface surface;
    WGPUTextureFormat surfaceFormat;
    WGPURenderPipeline pipeline;
    WGPUBuffer vertexBuffer;
    WGPUBuffer instanceBuffer;
    WGPUBuffer uniformBuffer;
    WGPUBindGroup bindGroup;
} state;

bool initialiseWGPU();
void wgpuTerminate();

void draw();