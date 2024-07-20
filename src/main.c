#include <stdio.h>
#include <stdbool.h>

#include <io.h>
#include <input.h>
#include <camera.h>

#include <renderer.h>

static uint64_t currentTime = 0;
static uint64_t previousTime = 0;
double deltaTime = 0;

void run() {
    previousTime = 0;
    currentTime = SDL_GetPerformanceCounter();
    deltaTime = 0;

    bool shouldClose = false;
    while (!shouldClose) {
        // Poll events and handle them.
        // (contrary to GLFW, close event is not automatically managed, and there
        // is no callback mechanism by default.)
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    shouldClose = true;
                    break;
                case SDL_KEYDOWN:
                    if (!event.key.repeat) {
                        keyboardStateBuffer[event.key.keysym.scancode] = true;
                    }
                    break;
                case SDL_KEYUP:
                    keyboardStateBuffer[event.key.keysym.scancode] = false;
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    mouseStateBuffer[event.button.button] = true;
                    break;
                case SDL_MOUSEBUTTONUP:
                    mouseStateBuffer[event.button.button] = false;
                    break;
                default:
                    break;
            }
        }

        if (getKey(SDL_SCANCODE_LEFT)) {
            cameraUniform.position[0] -= 0.005f;
        }
        if (getKey(SDL_SCANCODE_RIGHT)) {
            cameraUniform.position[0] += 0.005f;
        }
        if (getKey(SDL_SCANCODE_UP)) {
            cameraUniform.position[1] += 0.005f;
        }
        if (getKey(SDL_SCANCODE_DOWN)) {
            cameraUniform.position[1] -= 0.005f;
        }
        if (getKey(SDL_SCANCODE_W)) {
            cameraUniform.zoom += 0.005f;
        }
        if (getKey(SDL_SCANCODE_S)) {
            cameraUniform.zoom -= 0.005f;
        }

        previousTime = currentTime;
        currentTime = SDL_GetPerformanceCounter();
        deltaTime = ((currentTime - previousTime)*1000 / (double)SDL_GetPerformanceFrequency());

        draw();

        // INPUT UPDATES
        flushInput();
    }

    wgpuBufferDestroy(state.vertexBuffer);
    wgpuBufferRelease(state.vertexBuffer);

    //wgpuBufferDestroy(state.indexBuffer);
    //wgpuBufferRelease(state.indexBuffer);

    wgpuBufferDestroy(state.uniformBuffer);
    wgpuBufferRelease(state.uniformBuffer);

    //wgpuTextureDestroy(texture);
    //wgpuTextureRelease(texture);
}

int main() {
    int counter = 0;
    for (int i = 0; i < 10; ++i) {
        int hexCoordinates[2] = {counter, counter};
        hexToWorldCoordinates(hexCoordinates, instances[i].position);
        instances[i].size = 0.1f;
        counter++;
    }

    for (int i = 0; i < 10; ++i) {
        printf("%f %f\n", instances[i].position[0], instances[i].position[1]);
    }

    if(initialiseWGPU()) {
        run();
    }

    wgpuTerminate();

    return 0;
}