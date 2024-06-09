#include <stdio.h>
#include <stdbool.h>
#include <webgpu/wgpu.h>
#include <SDL2/SDL.h>

#include <io.h>
#include <sdl2webgpu.h>
#include <input.h>

float currentTime = 0.0f;

float vertexData[30] = {
    -0.5, -0.5, 1.0, 0.0, 0.0,
    +0.5, -0.5, 0.0, 1.0, 0.0,
    +0.5, +0.5, 0.0, 0.0, 1.0,
    -0.5, +0.5, 1.0, 0.0, 0.0,
};

uint16_t indexData[6] = {
    0,1,2,
    0,2,3,
};

int vertexCount = 6;

struct State {
    WGPUDevice device;
    WGPUQueue queue;
    SDL_Window* window;
    WGPUSurface surface;
    WGPUTextureFormat surfaceFormat;
    WGPURenderPipeline pipeline;
    WGPUBuffer vertexBuffer;
    WGPUBuffer indexBuffer;
    WGPUBuffer uniformBuffer;
    WGPUBindGroup bindGroup;
} state;

typedef struct {
    WGPUAdapter adapter;
    bool requestEnded;
} AdapterRequestData;

typedef struct {
    WGPUDevice device;
    bool requestEnded;
} DeviceRequestData;

void onAdapterRequestEnded(WGPURequestAdapterStatus status, WGPUAdapter adapter, const char* message, void* requestData) {
    AdapterRequestData* pRequestData = (AdapterRequestData*)requestData;

    if (status == WGPURequestAdapterStatus_Success) {
        pRequestData->adapter = adapter;
    } else {
        printf("Could not get WebGPU adapter\n");
    }

    pRequestData->requestEnded = true;
}

void onDeviceRequestEnded(WGPURequestDeviceStatus status, WGPUDevice device, const char* message, void* requestData) {
    DeviceRequestData* pRequestData = (DeviceRequestData*)requestData;

    if (status == WGPURequestAdapterStatus_Success) {
        pRequestData->device = device;
    } else {
        printf("Could not get WebGPU device\n");
    }

    pRequestData->requestEnded = true;
}

void onDeviceLost(WGPUDeviceLostReason reason, char const* message, void* requestData) {
    printf("Device lost\n");
    printf("%d\n", reason);
}

void onDeviceError(WGPUErrorType type, char const* message, void* requestData) {
    //printf("Uncaptured device error\n");
    //printf("%d\n", type);
}

void onQueueWorkDone(WGPUQueueWorkDoneStatus status, void* requestData) {
    printf("Queue work finished\n");
    printf("%d\n", status);
}

WGPURenderPipeline createRenderPipeline(WGPUPipelineLayout layout) {
    // LOAD SHADERS
    FileManager fileManager = createFileManager("shaders/main.wgsl");
    char shaderBuffer[fileManager.size + 1];
    fileManager.buffer = shaderBuffer;
    readFile(&fileManager);
    destroyFileManager(&fileManager);

    // COMPILE SHADERS
    WGPUShaderModuleWGSLDescriptor shaderCodeDescriptor = {};
    shaderCodeDescriptor.chain.next = NULL;
    shaderCodeDescriptor.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;
    shaderCodeDescriptor.code = shaderBuffer;

    WGPUShaderModuleDescriptor shaderModuleDescriptor = {};
    shaderModuleDescriptor.hintCount = 0;
    shaderModuleDescriptor.hints = NULL;
    shaderModuleDescriptor.nextInChain = &shaderCodeDescriptor.chain;

    WGPUShaderModule shaderModule = wgpuDeviceCreateShaderModule(state.device, &shaderModuleDescriptor);

    WGPURenderPipelineDescriptor pipelineDescriptor = {};

    // Vertex pipeline state
    WGPUVertexAttribute vertexAttributes[2] = {};
    vertexAttributes[0].shaderLocation = 0;
    vertexAttributes[0].format = WGPUVertexFormat_Float32x2;
    vertexAttributes[0].offset = 0;
    vertexAttributes[1].shaderLocation = 1;
    vertexAttributes[1].format = WGPUVertexFormat_Float32x3;
    vertexAttributes[1].offset = 2 * sizeof(float);

    WGPUVertexBufferLayout vertexBufferLayout = {};
    vertexBufferLayout.attributeCount = 2;
    vertexBufferLayout.attributes = vertexAttributes;
    vertexBufferLayout.arrayStride = 5 * sizeof(float);
    vertexBufferLayout.stepMode = WGPUVertexStepMode_Vertex;
    pipelineDescriptor.vertex.bufferCount = 1;
    pipelineDescriptor.vertex.buffers = &vertexBufferLayout;

    pipelineDescriptor.vertex.module = shaderModule;
    pipelineDescriptor.vertex.entryPoint = "vs_main";
    pipelineDescriptor.vertex.constantCount = 0;
    pipelineDescriptor.vertex.constants = NULL;

    // primitive pipeline state
    // each sequence of 3 points is a triangle
    // TODO: USE TRIANGLE STRIPS TO SAVE RENDERING
    pipelineDescriptor.primitive.topology = WGPUPrimitiveTopology_TriangleList;
    // which order vertices should be connected
    pipelineDescriptor.primitive.stripIndexFormat = WGPUIndexFormat_Undefined;
    pipelineDescriptor.primitive.frontFace = WGPUFrontFace_CCW;

    // TODO: ONLY RENDER THE FRONT TO AVOID WASTING RESOURCES BY RENDERING THE TRIANGLE TWICE
    pipelineDescriptor.primitive.cullMode = WGPUCullMode_None;

    // fragment pipeline state
    WGPUFragmentState fragmentState = {};
    fragmentState.module = shaderModule;
    fragmentState.entryPoint = "fs_main";
    fragmentState.constantCount = 0;
    fragmentState.constants = NULL;

    // configure the blending stage here:
    WGPUBlendState blendState = {};
    blendState.color.srcFactor = WGPUBlendFactor_SrcAlpha;
    blendState.color.dstFactor = WGPUBlendFactor_OneMinusSrcAlpha;
    blendState.color.operation = WGPUBlendOperation_Add;

    blendState.alpha.srcFactor = WGPUBlendFactor_Zero;
    blendState.alpha.dstFactor = WGPUBlendFactor_One;
    blendState.alpha.operation = WGPUBlendOperation_Add;

    WGPUColorTargetState colorTarget = {};
    colorTarget.format = state.surfaceFormat;
    colorTarget.blend = &blendState;

    // We could write to only some of the color channels
    colorTarget.writeMask = WGPUColorWriteMask_All; 
    fragmentState.targetCount = 1;
    fragmentState.targets = &colorTarget;

    pipelineDescriptor.fragment = &fragmentState;

    // stencil/deptch pipeline state
    pipelineDescriptor.depthStencil = NULL;

    // multi-sampling state
    pipelineDescriptor.multisample.count = 1;
    // default value for the mask, meaning all bits on
    pipelineDescriptor.multisample.mask = ~0u;
    // default as well (irrelevant for count = 1 anyway)
    pipelineDescriptor.multisample.alphaToCoverageEnabled = false;

    pipelineDescriptor.layout = layout;
    WGPURenderPipeline pipeline = wgpuDeviceCreateRenderPipeline(state.device, &pipelineDescriptor);
        
    wgpuShaderModuleRelease(shaderModule);

    return pipeline;
}

bool initialiseSDL2() {
    SDL_SetMainReady();
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Error in initialising SDL %s\n", SDL_GetError());
        return false;
    }
    
    // set window parameters
    int windowFlags = 0;
    SDL_Window* window = SDL_CreateWindow("Learn WebGPU", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, windowFlags);
    if (!window) {
        fprintf(stderr, "Failed to create SDL2 window\n");
        SDL_Quit();
        return false;
    }

    state.window = window;

    return true;
}

void initialiseBuffers() {
    WGPUBufferDescriptor bufferDescriptor = {};
    bufferDescriptor.nextInChain = NULL;
    bufferDescriptor.mappedAtCreation = false;

    // VERTEX BUFFER
    bufferDescriptor.size = sizeof(vertexData);
    bufferDescriptor.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Vertex;
    WGPUBuffer vertexBuffer = wgpuDeviceCreateBuffer(state.device, &bufferDescriptor);
    wgpuQueueWriteBuffer(state.queue, vertexBuffer, 0, vertexData, bufferDescriptor.size);

    // INDEX BUFFER
    bufferDescriptor.size = sizeof(indexData);
    // round it up to the nearest multiple of 4 bytes
    bufferDescriptor.size = (bufferDescriptor.size + 3) & ~3;
    bufferDescriptor.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Index;
    WGPUBuffer indexBuffer = wgpuDeviceCreateBuffer(state.device, &bufferDescriptor);
    wgpuQueueWriteBuffer(state.queue, indexBuffer, 0, indexData, bufferDescriptor.size);

    // UNIFORM BUFFER
    bufferDescriptor.size = sizeof(float);
    bufferDescriptor.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Uniform;
    WGPUBuffer uniformBuffer = wgpuDeviceCreateBuffer(state.device, &bufferDescriptor);
    wgpuQueueWriteBuffer(state.queue, uniformBuffer, 0, &currentTime, sizeof(float));

    state.vertexBuffer = vertexBuffer;
    state.indexBuffer = indexBuffer;
    state.uniformBuffer = uniformBuffer;
}

bool initialiseWGPU() {
    if(!initialiseSDL2()) {
        printf("Could not initialise SDL");
    }
    // reuse variable for success of various requests
    bool success;

    // INSTANCE
    WGPUInstance instance = wgpuCreateInstance(NULL);
    if (!instance) {
        printf("Could not initialize WebGPU\n");
        return 1;
    }

    // ADAPTER
    WGPUAdapter adapter;
    AdapterRequestData adapterRequestData = {};
    wgpuInstanceRequestAdapter(instance, NULL, onAdapterRequestEnded, (void*)&adapterRequestData);
    adapter = adapterRequestData.adapter;

	WGPUSurface surface = SDL_GetWGPUSurface(instance, state.window);
	printf("surface = %p\n", surface);

    if (!surface) {
        printf("Could not get WGPU surface\n");
        return 1;
    }

    WGPUSupportedLimits adapterLimits;
    adapterLimits.nextInChain = NULL;

    success = wgpuAdapterGetLimits(adapter, &adapterLimits);
    if (success) {
        printf("Adapter limits:\n");
        printf("maxTextureDimension1D: %d\n", adapterLimits.limits.maxTextureDimension1D);
        printf("maxTextureDimension2D: %d\n", adapterLimits.limits.maxTextureDimension2D);
        printf("maxTextureDimension3D: %d\n", adapterLimits.limits.maxTextureDimension3D);
        printf("maxTextureArrayLayers: %d\n", adapterLimits.limits.maxTextureArrayLayers);
    }

    // DEVICE
    WGPUDevice device;
    // TODO: SET DEVICE LIMITS
    WGPUDeviceDescriptor deviceDescriptor = {};
    deviceDescriptor.nextInChain = NULL;
    deviceDescriptor.label = "My Device";
    deviceDescriptor.requiredFeatureCount = 0; // we do not require any specific feature
    deviceDescriptor.requiredLimits = NULL; // we do not require any specific limit
    deviceDescriptor.defaultQueue.nextInChain = NULL;
    deviceDescriptor.defaultQueue.label = "The default queue";

    deviceDescriptor.deviceLostCallback = onDeviceLost;

    DeviceRequestData deviceRequestData = {};
    wgpuAdapterRequestDevice(adapter, NULL, onDeviceRequestEnded, (void*)&deviceRequestData);
    
    device = deviceRequestData.device;

    wgpuDeviceSetUncapturedErrorCallback(device, onDeviceError, NULL);

    WGPUSupportedLimits deviceLimits = {};
    deviceLimits.nextInChain = NULL;
    success = wgpuDeviceGetLimits(device, &deviceLimits);
    if (success) {
        printf("Device limits:\n");
        printf("maxTextureDimension1D: %d\n", deviceLimits.limits.maxTextureDimension1D);
        printf("maxTextureDimension2D: %d\n", deviceLimits.limits.maxTextureDimension2D);
        printf("maxTextureDimension3D: %d\n", deviceLimits.limits.maxTextureDimension3D);
        printf("maxTextureArrayLayers: %d\n", deviceLimits.limits.maxTextureArrayLayers);
    }

    // COMMAND QUEUE
    WGPUQueue queue = wgpuDeviceGetQueue(device);
    wgpuQueueOnSubmittedWorkDone(queue, onQueueWorkDone, NULL);

    WGPUSurfaceConfiguration surfaceConfiguration;
    surfaceConfiguration.nextInChain = NULL;
    surfaceConfiguration.width = 640;
    surfaceConfiguration.height = 480;
    WGPUTextureFormat surfaceFormat = wgpuSurfaceGetPreferredFormat(surface, adapter);
    surfaceConfiguration.format = surfaceFormat;
    surfaceConfiguration.viewFormatCount = 0;
    surfaceConfiguration.viewFormats = NULL;
    surfaceConfiguration.usage = WGPUTextureUsage_RenderAttachment;
    surfaceConfiguration.device = device;
    surfaceConfiguration.presentMode = WGPUPresentMode_Fifo;
    surfaceConfiguration.alphaMode = WGPUCompositeAlphaMode_Auto;
    wgpuSurfaceConfigure(surface, &surfaceConfiguration);

    state.device = device;
    state.queue = queue;
    state.surface = surface;
    state.surfaceFormat = surfaceFormat;

    initialiseBuffers();

    // binding layout
    WGPUBindGroupLayoutEntry bindingLayout = {};
    // SET DEFAULT (sets others to undefined so that only the resource we need is used)
    bindingLayout.buffer.nextInChain = NULL;
    bindingLayout.buffer.type = WGPUBufferBindingType_Undefined;
    bindingLayout.buffer.hasDynamicOffset = false;

    bindingLayout.sampler.nextInChain = NULL;
    bindingLayout.sampler.type = WGPUSamplerBindingType_Undefined;

    bindingLayout.storageTexture.nextInChain = NULL;
    bindingLayout.storageTexture.access = WGPUStorageTextureAccess_Undefined;
    bindingLayout.storageTexture.format = WGPUTextureFormat_Undefined;
    bindingLayout.storageTexture.viewDimension = WGPUTextureViewDimension_Undefined;

    bindingLayout.texture.nextInChain = NULL;
    bindingLayout.texture.multisampled = false;
    bindingLayout.texture.sampleType = WGPUTextureSampleType_Undefined;
    bindingLayout.texture.viewDimension = WGPUTextureViewDimension_Undefined;
    // END SET DEFAULT
    bindingLayout.binding = 0;
    bindingLayout.visibility = WGPUShaderStage_Vertex;
    bindingLayout.buffer.type = WGPUBufferBindingType_Uniform;
    bindingLayout.buffer.minBindingSize = sizeof(float);
    // pipeline layout
    WGPUBindGroupLayoutDescriptor bindGroupLayoutDescriptor = {};
    bindGroupLayoutDescriptor.entryCount = 1;
    bindGroupLayoutDescriptor.entries = &bindingLayout;
    WGPUBindGroupLayout bindGroupLayout = wgpuDeviceCreateBindGroupLayout(state.device, &bindGroupLayoutDescriptor);
    WGPUPipelineLayoutDescriptor pipelineLayoutDescriptor = {};
    pipelineLayoutDescriptor.nextInChain = NULL;
    pipelineLayoutDescriptor.bindGroupLayoutCount = 1;
    pipelineLayoutDescriptor.bindGroupLayouts = &bindGroupLayout;
    WGPUPipelineLayout layout = wgpuDeviceCreatePipelineLayout(state.device, &pipelineLayoutDescriptor);
    // Create a binding
	WGPUBindGroupEntry binding = {};
	// The index of the binding (the entries in bindGroupDescriptor can be in any order)
	binding.binding = 0;
	// The buffer it is actually bound to
	binding.buffer = state.uniformBuffer;
	// We can specify an offset within the buffer, so that a single buffer can hold
	// multiple uniform blocks.
	binding.offset = 0;
	// And we specify again the size of the buffer.
	binding.size = sizeof(float);
	// A bind group contains one or multiple bindings
	WGPUBindGroupDescriptor bindGroupDescriptor = {};
	bindGroupDescriptor.layout = bindGroupLayout;
	// There must be as many bindings as declared in the layout!
	bindGroupDescriptor.entryCount = 1;
	bindGroupDescriptor.entries = &binding;
	WGPUBindGroup bindGroup = wgpuDeviceCreateBindGroup(state.device, &bindGroupDescriptor);

    state.bindGroup = bindGroup;
    state.pipeline = createRenderPipeline(layout);

    wgpuAdapterRelease(adapter);
    wgpuInstanceRelease(instance);
    
    return true;
}

void terminate() {
    // free wgpu resources
    wgpuRenderPipelineRelease(state.pipeline);
    wgpuSurfaceUnconfigure(state.surface);
    wgpuSurfaceRelease(state.surface);
    wgpuQueueRelease(state.queue);
    wgpuDeviceRelease(state.device);
    wgpuBindGroupRelease(state.bindGroup);

    // free SDL2 resources
    SDL_DestroyWindow(state.window);
    SDL_Quit();
}

WGPUTextureView getNextSurfaceTextureView() {
    WGPUSurfaceTexture surfaceTexture;
    wgpuSurfaceGetCurrentTexture(state.surface, &surfaceTexture);

    if (surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_Success) {
        return NULL;
    }

    WGPUTextureViewDescriptor viewDescriptor;
    viewDescriptor.nextInChain = NULL;
    viewDescriptor.label = "Surface texture view";
    viewDescriptor.format = wgpuTextureGetFormat(surfaceTexture.texture);
    viewDescriptor.dimension = WGPUTextureViewDimension_2D;
    viewDescriptor.baseMipLevel = 0;
    viewDescriptor.mipLevelCount = 1;
    viewDescriptor.baseArrayLayer = 0;
    viewDescriptor.arrayLayerCount = 1;
    viewDescriptor.aspect = WGPUTextureAspect_All;
    WGPUTextureView targetView = wgpuTextureCreateView(surfaceTexture.texture, &viewDescriptor);

    return targetView;
}

void run() {

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
            currentTime -= 0.005f;
        }
        else if (getKey(SDL_SCANCODE_RIGHT)) {
            currentTime += 0.005f;
        }

        wgpuQueueWriteBuffer(state.queue, state.uniformBuffer, 0, &currentTime, sizeof(float));

        // Get the next target texture view
        WGPUTextureView targetView = getNextSurfaceTextureView();
        if (!targetView) return;

        // Create a command encoder for the draw call
        WGPUCommandEncoderDescriptor encoderDescriptor = {};
        encoderDescriptor.nextInChain = NULL;
        encoderDescriptor.label = "My command encoder";
        WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(state.device, &encoderDescriptor);

        // Create the render pass that clears the screen with our color
        WGPURenderPassDescriptor renderPassDesc = {};
        renderPassDesc.nextInChain = NULL;

        // The attachment part of the render pass descriptor describes the target texture of the pass
        WGPURenderPassColorAttachment renderPassColorAttachment = {};
        renderPassColorAttachment.view = targetView;
        renderPassColorAttachment.resolveTarget = NULL;
        renderPassColorAttachment.loadOp = WGPULoadOp_Clear;
        renderPassColorAttachment.storeOp = WGPUStoreOp_Store;
        renderPassColorAttachment.clearValue = (WGPUColor){0.9, 0.1, 0.2, 1.0};
        renderPassColorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;

        renderPassDesc.colorAttachmentCount = 1;
        renderPassDesc.colorAttachments = &renderPassColorAttachment;
        renderPassDesc.depthStencilAttachment = NULL;
        renderPassDesc.timestampWrites = NULL;

        // Create the render pass
        WGPURenderPassEncoder renderPass = wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDesc);
        wgpuRenderPassEncoderSetPipeline(renderPass, state.pipeline);

        wgpuRenderPassEncoderSetVertexBuffer(renderPass, 0, state.vertexBuffer, 0, sizeof(vertexData));
        wgpuRenderPassEncoderSetIndexBuffer(renderPass, state.indexBuffer, WGPUIndexFormat_Uint16, 0, sizeof(indexData));

        // Draw to the screen
        wgpuRenderPassEncoderSetBindGroup(renderPass, 0, state.bindGroup, 0, NULL);
        wgpuRenderPassEncoderDrawIndexed(renderPass, 6, 1, 0, 0, 0);

        wgpuRenderPassEncoderEnd(renderPass);
        wgpuRenderPassEncoderRelease(renderPass);

        // Finally encode and submit the render pass
        WGPUCommandBufferDescriptor commandBufferDescriptor = {};
        commandBufferDescriptor.nextInChain = NULL;
        commandBufferDescriptor.label = "Command buffer";
        WGPUCommandBuffer command = wgpuCommandEncoderFinish(encoder, &commandBufferDescriptor);
        wgpuCommandEncoderRelease(encoder);

        wgpuQueueSubmit(state.queue, 1, &command);
        wgpuCommandBufferRelease(command);

        // At the end of the frame
        wgpuTextureViewRelease(targetView);
        wgpuSurfacePresent(state.surface);

        wgpuDevicePoll(state.device, false, NULL);
        
        // INPUT UPDATES
        flushInput();
    }

    wgpuBufferDestroy(state.vertexBuffer);
    wgpuBufferRelease(state.vertexBuffer);

    wgpuBufferDestroy(state.indexBuffer);
    wgpuBufferRelease(state.indexBuffer);

    wgpuBufferDestroy(state.uniformBuffer);
    wgpuBufferRelease(state.uniformBuffer);
}

int main() {

    if(initialiseWGPU()) {
        run();
    }

    terminate();

    return 0;
}

