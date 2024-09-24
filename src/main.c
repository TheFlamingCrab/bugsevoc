#include <stdio.h>
#include <stdbool.h>

#include <io.h>
#include <input.h>
#include <camera.h>

#include <renderer.h>

static uint64_t currentTime = 0;
static uint64_t previousTime = 0;

// Time since last frame in milliseconds
double deltaTime = 0;

// INFO, Vsync is on right now by default, so the frame rate should be relatively constant

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
                case SDL_MOUSEMOTION:
                    mousePositionBuffer[0] = event.motion.x;
                    mousePositionBuffer[1] = event.motion.y;
                default:
                    break;
            }
        }

        //TODO: ADD GAME LOGIC HERE

        //INPUT EXAMPLES

        // x and y coordinates of mouse
        //printf("%f\n", mousePositionBuffer[0]);
        //printf("%f\n", mousePositionBuffer[1]);

        // returns true if left arrow key is down
        /*if (getKey(SDL_SCANCODE_LEFT)) {
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
        }*/
        // returns true once on the frame that the W key is released
        /*if (getKeyUp(SDL_SCANCODE_W)) {
            // ZOOM IN (the zoom is really bad but it kind of works)
            cameraUniform.zoom += 0.005f;
        }
        // returns true once on the frame that the S key is pressed
        if (getKeyDown(SDL_SCANCODE_S)) {
            // ZOOM OUT (self explanatory)
            cameraUniform.zoom -= 0.005f;
        }*/
        // returns true every frame while left click is held
        // if (getMouseButton(SDL_BUTTON_LEFT)) {
        //  etc
        // }

        // EXAMPLE OF READING IN A FILE
        /*
        FileManager fileManager = createFileManager("shaders/main.wgsl");
        char shaderBuffer[fileManager.size + 1];
        fileManager.buffer = shaderBuffer;
        readFile(&fileManager);
        destroyFileManager(&fileManager); 
        
        and now we have a char array with the contents of the file! yippee!!
        */

        // DELTA TIME EXAMPLES (time between last and next frame in milliseconds) (this is pseudocode)
        // access via deltaTime variable
        // Move(5 * deltaTime, left)

        int width = 128;
        int height = 128;
        int radius = 10;

        if (getMouseButton(SDL_BUTTON_LEFT)) {
            // TODO: MAKE THIS NOT ABSOLUTELY AWFUL

            int x_pixel = round(mousePositionBuffer[0] * (128.0f / 640.0f));
            int y_pixel = 128 - round(mousePositionBuffer[1] * (128.0f / 480.0f));

            for (int i = 0 ; i < width; ++i) {
                for (int j = 0; j < height; ++j) {
                    float distance = sqrtf(powf(x_pixel - i, 2) + powf(y_pixel - j, 2));

                    if (distance < radius) {
                        // edit the pixels
                        uint8_t* p = &pixels[4 * (j * width + i)];
                        p[0] = 0;
                        p[1] = 0;
                        p[2] = 0;
                        p[3] = 255;
                    }
                }
            }
        }

        // DONT MESS WITH THIS ONWARDS

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
    if(initialiseWGPU()) {
        run();
    }

    wgpuTerminate();

    return 0;
}