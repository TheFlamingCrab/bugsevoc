#include <webgpu/wgpu.h>
#include <SDL2/SDL.h>
#include <sdl2webgpu.h>

#include <io.h>
#include <camera.h>
#include <input.h>

#include <renderer.h>

// honestly dont even bother trying to understand this, just trust that it works

WGPUSampler sampler;
WGPUBindGroupLayout bindGroupLayout;
WGPUTextureView textureView;

/*float vertexData[30] = {
    -1.0f, -1.0f, 1.0f, 0.0f, 0.0f,
    +1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    +1.0f, +1.0f, 0.0f, 0.0f, 1.0f,
    -1.0f, -1.0f, 1.0f, 0.0f, 0.0f,
    +1.0f, +1.0f, 0.0f, 0.0f, 1.0f,
    -1.0f, +1.0f, 1.0f, 0.0f, 0.0f,
};*/

float vertexData[12] = {
    -1.0f, -1.0f,
    +1.0f, -1.0f,
    +1.0f, +1.0f,
    -1.0f, -1.0f,
    +1.0f, +1.0f,
    -1.0f, +1.0f,
};

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
    printf("Uncaptured device error\n");
    printf("%d\n", type);
    exit(0);
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

    /*// Vertex pipeline state
    WGPUVertexBufferLayout vertexBufferLayouts[2];

    WGPUVertexAttribute vertexAttributes[2] = {};
    vertexAttributes[0].shaderLocation = 0;
    vertexAttributes[0].format = WGPUVertexFormat_Float32x2;
    vertexAttributes[0].offset = 0;
    vertexAttributes[1].shaderLocation = 1;
    vertexAttributes[1].format = WGPUVertexFormat_Float32x3;
    vertexAttributes[1].offset = 2 * sizeof(float);

    vertexBufferLayouts[0].attributeCount = 2;
    vertexBufferLayouts[0].attributes = vertexAttributes;
    vertexBufferLayouts[0].arrayStride = 5 * sizeof(float);
    vertexBufferLayouts[0].stepMode = WGPUVertexStepMode_Vertex;

    WGPUVertexAttribute instanceVertexAttributes[2] = {};
    instanceVertexAttributes[0].shaderLocation = 2;
    instanceVertexAttributes[0].format = WGPUVertexFormat_Float32x2;
    instanceVertexAttributes[0].offset = 0;
    instanceVertexAttributes[1].shaderLocation = 3;
    instanceVertexAttributes[1].format = WGPUVertexFormat_Float32;
    instanceVertexAttributes[1].offset = 2 * sizeof(float);

    vertexBufferLayouts[1].attributeCount = 2;
    vertexBufferLayouts[1].attributes = instanceVertexAttributes;
    vertexBufferLayouts[1].arrayStride = 3 * sizeof(float);
    vertexBufferLayouts[1].stepMode = WGPUVertexStepMode_Instance;

    pipelineDescriptor.vertex.bufferCount = 2;
    pipelineDescriptor.vertex.buffers = (WGPUVertexBufferLayout*)&vertexBufferLayouts;

    pipelineDescriptor.vertex.module = shaderModule;
    pipelineDescriptor.vertex.entryPoint = "vs_main";
    pipelineDescriptor.vertex.constantCount = 0;
    pipelineDescriptor.vertex.constants = NULL;*/

    WGPUVertexBufferLayout vertexBufferLayouts[1];

    WGPUVertexAttribute vertexAttributes[2] = {};
    vertexAttributes[0].shaderLocation = 0;
    vertexAttributes[0].format = WGPUVertexFormat_Float32x2;
    vertexAttributes[0].offset = 0;
    //vertexAttributes[1].shaderLocation = 1;
    //vertexAttributes[1].format = WGPUVertexFormat_Float32x3;
    //vertexAttributes[1].offset = 2 * sizeof(float);

    vertexBufferLayouts[0].attributeCount = 1;
    vertexBufferLayouts[0].attributes = vertexAttributes;
    vertexBufferLayouts[0].arrayStride = 2 * sizeof(float);
    vertexBufferLayouts[0].stepMode = WGPUVertexStepMode_Vertex;

    /*WGPUVertexAttribute instanceVertexAttributes[2] = {};
    instanceVertexAttributes[0].shaderLocation = 2;
    instanceVertexAttributes[0].format = WGPUVertexFormat_Float32x2;
    instanceVertexAttributes[0].offset = 0;
    instanceVertexAttributes[1].shaderLocation = 3;
    instanceVertexAttributes[1].format = WGPUVertexFormat_Float32;
    instanceVertexAttributes[1].offset = 2 * sizeof(float);

    vertexBufferLayouts[1].attributeCount = 2;
    vertexBufferLayouts[1].attributes = instanceVertexAttributes;
    vertexBufferLayouts[1].arrayStride = 3 * sizeof(float);
    vertexBufferLayouts[1].stepMode = WGPUVertexStepMode_Instance;*/

    pipelineDescriptor.vertex.bufferCount = 1;
    pipelineDescriptor.vertex.buffers = (WGPUVertexBufferLayout*)&vertexBufferLayouts;

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

void updateTexture() {
    // Create a binding
	WGPUBindGroupEntry bindings[3] = {};
	// The index of the binding (the entries in bindGroupDescriptor can be in any order)
	bindings[0].binding = 0;
	// The buffer it is actually bound to
	bindings[0].buffer = state.uniformBuffer;

	// We can specify an offset within the buffer, so that a single buffer can hold
	// multiple uniform blocks.
	bindings[0].offset = 0;
	// And we specify again the size of the buffer.
	bindings[0].size = sizeof(cameraUniform);

    bindings[1].binding = 1;
    bindings[1].textureView = textureView;

    bindings[2].binding = 2;
    bindings[2].sampler = sampler;

    // A bind group contains one or multiple bindings
	WGPUBindGroupDescriptor bindGroupDescriptor = {};
	bindGroupDescriptor.layout = bindGroupLayout;
	// There must be as many bindings as declared in the layout!
	bindGroupDescriptor.entryCount = 3;
	bindGroupDescriptor.entries = (WGPUBindGroupEntry*)&bindings;
	WGPUBindGroup bindGroup = wgpuDeviceCreateBindGroup(state.device, &bindGroupDescriptor);
    state.bindGroup = bindGroup;
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

    // INSTANCE BUFFER
    /*instances[0].position[0] = 0;
    instances[0].position[1] = 0;
    instances[0].size = 1.0f;
    instances[1].position[0] = 0.5f;
    instances[1].position[1] = 0.5f;
    instances[1].size = 0.75f;*/

    /*instances[0].position[0] = 0;
    instances[0].position[1] = 0;
    instances[0].size = 1.0f;
    bufferDescriptor.size = sizeof(instances);
    bufferDescriptor.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Vertex;
    WGPUBuffer instanceBuffer = wgpuDeviceCreateBuffer(state.device, &bufferDescriptor);
    wgpuQueueWriteBuffer(state.queue, instanceBuffer, 0, instances, bufferDescriptor.size);*/

    /*// INDEX BUFFER
    bufferDescriptor.size = sizeof(indexData);
    // round it up to the nearest multiple of 4 bytes
    bufferDescriptor.size = (bufferDescriptor.size + 3) & ~3;
    bufferDescriptor.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Index;
    WGPUBuffer indexBuffer = wgpuDeviceCreateBuffer(state.device, &bufferDescriptor);
    wgpuQueueWriteBuffer(state.queue, indexBuffer, 0, indexData, bufferDescriptor.size);*/

    // UNIFORM BUFFER
    cameraUniform.position[0] = 0.0f;
    cameraUniform.position[1] = 0.0f;
    cameraUniform.zoom = 1.0f;
    bufferDescriptor.size = sizeof(cameraUniform);
    bufferDescriptor.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Uniform;
    WGPUBuffer uniformBuffer = wgpuDeviceCreateBuffer(state.device, &bufferDescriptor);
    wgpuQueueWriteBuffer(state.queue, uniformBuffer, 0, &cameraUniform, sizeof(cameraUniform));

    state.vertexBuffer = vertexBuffer;
    //state.instanceBuffer = instanceBuffer;
    //state.indexBuffer = indexBuffer;
    state.uniformBuffer = uniformBuffer;
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
	//printf("surface = %p\n", surface);

    if (!surface) {
        printf("Could not get WGPU surface\n");
        return 1;
    }

    WGPUSupportedLimits adapterLimits;
    adapterLimits.nextInChain = NULL;

    success = wgpuAdapterGetLimits(adapter, &adapterLimits);
    if (success) {
        /*printf("Adapter limits:\n");
        printf("maxTextureDimension1D: %d\n", adapterLimits.limits.maxTextureDimension1D);
        printf("maxTextureDimension2D: %d\n", adapterLimits.limits.maxTextureDimension2D);
        printf("maxTextureDimension3D: %d\n", adapterLimits.limits.maxTextureDimension3D);
        printf("maxTextureArrayLayers: %d\n", adapterLimits.limits.maxTextureArrayLayers);*/
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
        /*printf("Device limits:\n");
        printf("maxTextureDimension1D: %d\n", deviceLimits.limits.maxTextureDimension1D);
        printf("maxTextureDimension2D: %d\n", deviceLimits.limits.maxTextureDimension2D);
        printf("maxTextureDimension3D: %d\n", deviceLimits.limits.maxTextureDimension3D);
        printf("maxTextureArrayLayers: %d\n", deviceLimits.limits.maxTextureArrayLayers);*/
    }

    // COMMAND QUEUE
    WGPUQueue queue = wgpuDeviceGetQueue(device);
    wgpuQueueOnSubmittedWorkDone(queue, onQueueWorkDone, NULL);

    WGPUSurfaceConfiguration surfaceConfiguration = {};
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

    uint32_t mipLevelCount = 1;

    // CREATE TEXTURE
    WGPUTextureDescriptor textureDescriptor = {};
    textureDescriptor.dimension = WGPUTextureDimension_2D;
    textureDescriptor.nextInChain = NULL;
    textureDescriptor.size = (WGPUExtent3D){128, 128, 1};
    textureDescriptor.mipLevelCount = mipLevelCount;
    textureDescriptor.sampleCount = 1;
    textureDescriptor.format = WGPUTextureFormat_RGBA8Unorm;
    textureDescriptor.usage = WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst;
    textureDescriptor.viewFormatCount = 0;
    textureDescriptor.viewFormats = NULL;
    WGPUTexture texture = wgpuDeviceCreateTexture(state.device, &textureDescriptor);

    for (int i = 0 ; i < textureDescriptor.size.width; ++i) {
        for (int j = 0; j < textureDescriptor.size.height; ++j) {
            uint8_t* p = &pixels[4 * (j * textureDescriptor.size.width + i)];
            p[0] = (uint8_t)i;
            p[1] = (uint8_t)j;
            p[2] = 128;
            p[3] = 255;
        }
    }
    
    WGPUImageCopyTexture destination;
    destination.texture = texture;
    destination.mipLevel = 0;
    destination.origin = (WGPUOrigin3D){0, 0, 0};
    destination.aspect = WGPUTextureAspect_All;

    WGPUTextureDataLayout source;
    source.offset = 0;
    // minimum of 256 bytes
    source.bytesPerRow = 4 * textureDescriptor.size.width;
    source.rowsPerImage = textureDescriptor.size.height;

    wgpuQueueWriteTexture(state.queue, &destination, pixels, 4 * textureDescriptor.size.width * textureDescriptor.size.height, &source, &textureDescriptor.size);

    WGPUTextureViewDescriptor textureViewDescriptor = {};
    textureViewDescriptor.aspect = WGPUTextureAspect_All;
    textureViewDescriptor.baseArrayLayer = 0;
    textureViewDescriptor.arrayLayerCount = 1;
    textureViewDescriptor.baseMipLevel = 0;
    textureViewDescriptor.mipLevelCount = mipLevelCount;
    textureViewDescriptor.dimension = WGPUTextureViewDimension_2D;
    textureViewDescriptor.format = textureDescriptor.format;
    textureView = wgpuTextureCreateView(texture, &textureViewDescriptor);
    // TODO: RELEASE TEXTURE VIEW
    // END CREATE TEXTURE

    WGPUSamplerDescriptor samplerDescriptor = {};
    samplerDescriptor.addressModeU = WGPUAddressMode_ClampToEdge;
    samplerDescriptor.addressModeV = WGPUAddressMode_ClampToEdge;
    samplerDescriptor.addressModeW = WGPUAddressMode_ClampToEdge;
    samplerDescriptor.magFilter = WGPUFilterMode_Linear;
    samplerDescriptor.minFilter = WGPUFilterMode_Linear;
    samplerDescriptor.mipmapFilter = WGPUMipmapFilterMode_Linear;
    samplerDescriptor.lodMinClamp = 0.0f;
    samplerDescriptor.lodMaxClamp = 1.0f;
    samplerDescriptor.compare = WGPUCompareFunction_Undefined;
    samplerDescriptor.maxAnisotropy = 1;

    sampler = wgpuDeviceCreateSampler(device, &samplerDescriptor);

    // binding layout
    WGPUBindGroupLayoutEntry bindingLayouts[3] = {};
    // SET DEFAULT (sets others to undefined so that only the resource we need are used)
    bindingLayouts[0].buffer.nextInChain = NULL;
    bindingLayouts[0].buffer.type = WGPUBufferBindingType_Undefined;
    bindingLayouts[0].buffer.hasDynamicOffset = false;

    bindingLayouts[0].sampler.nextInChain = NULL;
    bindingLayouts[0].sampler.type = WGPUSamplerBindingType_Undefined;

    bindingLayouts[0].storageTexture.nextInChain = NULL;
    bindingLayouts[0].storageTexture.access = WGPUStorageTextureAccess_Undefined;
    bindingLayouts[0].storageTexture.format = WGPUTextureFormat_Undefined;
    bindingLayouts[0].storageTexture.viewDimension = WGPUTextureViewDimension_Undefined;

    bindingLayouts[0].texture.nextInChain = NULL;
    bindingLayouts[0].texture.multisampled = false;
    bindingLayouts[0].texture.sampleType = WGPUTextureSampleType_Undefined;
    bindingLayouts[0].texture.viewDimension = WGPUTextureViewDimension_Undefined;

    // ANOTHER ROUND
    bindingLayouts[1].buffer.nextInChain = NULL;
    bindingLayouts[1].buffer.type = WGPUBufferBindingType_Undefined;
    bindingLayouts[1].buffer.hasDynamicOffset = false;

    bindingLayouts[1].sampler.nextInChain = NULL;
    bindingLayouts[1].sampler.type = WGPUSamplerBindingType_Undefined;

    bindingLayouts[1].storageTexture.nextInChain = NULL;
    bindingLayouts[1].storageTexture.access = WGPUStorageTextureAccess_Undefined;
    bindingLayouts[1].storageTexture.format = WGPUTextureFormat_Undefined;
    bindingLayouts[1].storageTexture.viewDimension = WGPUTextureViewDimension_Undefined;

    bindingLayouts[1].texture.nextInChain = NULL;
    bindingLayouts[1].texture.multisampled = false;
    bindingLayouts[1].texture.sampleType = WGPUTextureSampleType_Undefined;
    bindingLayouts[1].texture.viewDimension = WGPUTextureViewDimension_Undefined;

    bindingLayouts[2].buffer.nextInChain = NULL;
    bindingLayouts[2].buffer.type = WGPUBufferBindingType_Undefined;
    bindingLayouts[2].buffer.hasDynamicOffset = false;

    bindingLayouts[2].sampler.nextInChain = NULL;
    bindingLayouts[2].sampler.type = WGPUSamplerBindingType_Undefined;

    bindingLayouts[2].storageTexture.nextInChain = NULL;
    bindingLayouts[2].storageTexture.access = WGPUStorageTextureAccess_Undefined;
    bindingLayouts[2].storageTexture.format = WGPUTextureFormat_Undefined;
    bindingLayouts[2].storageTexture.viewDimension = WGPUTextureViewDimension_Undefined;

    bindingLayouts[2].texture.nextInChain = NULL;
    bindingLayouts[2].texture.multisampled = false;
    bindingLayouts[2].texture.sampleType = WGPUTextureSampleType_Undefined;
    bindingLayouts[2].texture.viewDimension = WGPUTextureViewDimension_Undefined;
    // END SET DEFAULT

    // UNIFORM BINDING LAYOUT
    bindingLayouts[0].binding = 0;
    bindingLayouts[0].visibility = WGPUShaderStage_Vertex;
    bindingLayouts[0].buffer.type = WGPUBufferBindingType_Uniform;
    bindingLayouts[0].buffer.minBindingSize = sizeof(cameraUniform);
    // TEXTURE BINDING LAYOUT
    bindingLayouts[1].nextInChain = NULL;
    bindingLayouts[1].binding = 1;
    bindingLayouts[1].visibility = WGPUShaderStage_Fragment;
    bindingLayouts[1].texture.sampleType = WGPUTextureSampleType_Float;
    bindingLayouts[1].texture.viewDimension = WGPUTextureViewDimension_2D;
    bindingLayouts[1].texture.multisampled = false;

    bindingLayouts[2].binding = 2;
    bindingLayouts[2].visibility = WGPUShaderStage_Fragment;
    bindingLayouts[2].sampler.type = WGPUSamplerBindingType_Filtering;
        
    // pipeline layout
    WGPUBindGroupLayoutDescriptor bindGroupLayoutDescriptor = {};
    bindGroupLayoutDescriptor.entryCount = 3;
    bindGroupLayoutDescriptor.entries = (WGPUBindGroupLayoutEntry*)&bindingLayouts;
    bindGroupLayout = wgpuDeviceCreateBindGroupLayout(state.device, &bindGroupLayoutDescriptor);
    WGPUPipelineLayoutDescriptor pipelineLayoutDescriptor = {};
    pipelineLayoutDescriptor.nextInChain = NULL;
    pipelineLayoutDescriptor.bindGroupLayoutCount = 1;
    pipelineLayoutDescriptor.bindGroupLayouts = &bindGroupLayout;
    WGPUPipelineLayout layout = wgpuDeviceCreatePipelineLayout(state.device, &pipelineLayoutDescriptor);

    /*// Create a binding
	WGPUBindGroupEntry bindings[3] = {};
	// The index of the binding (the entries in bindGroupDescriptor can be in any order)
	bindings[0].binding = 0;
	// The buffer it is actually bound to
	bindings[0].buffer = state.uniformBuffer;

	// We can specify an offset within the buffer, so that a single buffer can hold
	// multiple uniform blocks.
	bindings[0].offset = 0;
	// And we specify again the size of the buffer.
	bindings[0].size = sizeof(cameraUniform);

    bindings[1].binding = 1;
    bindings[1].textureView = textureView;

    bindings[2].binding = 2;
    bindings[2].sampler = sampler;

	// A bind group contains one or multiple bindings
	WGPUBindGroupDescriptor bindGroupDescriptor = {};
	bindGroupDescriptor.layout = bindGroupLayout;
	// There must be as many bindings as declared in the layout!
	bindGroupDescriptor.entryCount = 3;
	bindGroupDescriptor.entries = (WGPUBindGroupEntry*)&bindings;

	WGPUBindGroup bindGroup = wgpuDeviceCreateBindGroup(state.device, &bindGroupDescriptor);
        state.bindGroup = bindGroup;*/
    updateTexture();

    printf("hello\n");
    state.pipeline = createRenderPipeline(layout);
    printf("hello\n");

    wgpuPipelineLayoutRelease(layout);
    wgpuAdapterRelease(adapter);
    wgpuInstanceRelease(instance);

    return true;
}

void wgpuTerminate() {
    // free wgpu resources
    
    wgpuBindGroupLayoutRelease(bindGroupLayout);

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

    WGPUTextureViewDescriptor viewDescriptor = {};
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

void draw() {
    uint32_t mipLevelCount = 1;

    WGPUTextureDescriptor textureDescriptor = {};
    textureDescriptor.dimension = WGPUTextureDimension_2D;
    textureDescriptor.nextInChain = NULL;
    textureDescriptor.size = (WGPUExtent3D){128, 128, 1};
    textureDescriptor.mipLevelCount = mipLevelCount;
    textureDescriptor.sampleCount = 1;
    textureDescriptor.format = WGPUTextureFormat_RGBA8Unorm;
    textureDescriptor.usage = WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst;
    textureDescriptor.viewFormatCount = 0;
    textureDescriptor.viewFormats = NULL;
    WGPUTexture texture = wgpuDeviceCreateTexture(state.device, &textureDescriptor);
    
    WGPUImageCopyTexture destination;
    destination.texture = texture;
    destination.mipLevel = 0;
    destination.origin = (WGPUOrigin3D){0, 0, 0};
    destination.aspect = WGPUTextureAspect_All;

    WGPUTextureDataLayout source;
    source.offset = 0;
    // minimum of 256 bytes
    source.bytesPerRow = 4 * textureDescriptor.size.width;
    source.rowsPerImage = textureDescriptor.size.height;

    wgpuQueueWriteTexture(state.queue, &destination, pixels, 4 * textureDescriptor.size.width * textureDescriptor.size.height, &source, &textureDescriptor.size);

    WGPUTextureViewDescriptor textureViewDescriptor = {};
    textureViewDescriptor.aspect = WGPUTextureAspect_All;
    textureViewDescriptor.baseArrayLayer = 0;
    textureViewDescriptor.arrayLayerCount = 1;
    textureViewDescriptor.baseMipLevel = 0;
    textureViewDescriptor.mipLevelCount = mipLevelCount;
    textureViewDescriptor.dimension = WGPUTextureViewDimension_2D;
    textureViewDescriptor.format = textureDescriptor.format;
    textureView = wgpuTextureCreateView(texture, &textureViewDescriptor);
    // TODO: RELEASE TEXTURE VIEW
    // END CREATE TEXTURE

    updateTexture();

    // TODO: ABSTRACT TEXTURE PASSING
    wgpuQueueWriteBuffer(state.queue, state.uniformBuffer, 0, &cameraUniform, sizeof(cameraUniform));

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
    renderPassColorAttachment.clearValue = (WGPUColor){0.9f, 0.1f, 0.2f, 1.0f};
    renderPassColorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;

    renderPassDesc.colorAttachmentCount = 1;
    renderPassDesc.colorAttachments = &renderPassColorAttachment;
    renderPassDesc.depthStencilAttachment = NULL;
    renderPassDesc.timestampWrites = NULL;

    // Create the render pass
    WGPURenderPassEncoder renderPass = wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDesc);
    wgpuRenderPassEncoderSetPipeline(renderPass, state.pipeline);

    wgpuRenderPassEncoderSetVertexBuffer(renderPass, 0, state.vertexBuffer, 0, sizeof(vertexData));
    //wgpuRenderPassEncoderSetVertexBuffer(renderPass, 1, state.instanceBuffer, 0, sizeof(instances));
    //wgpuRenderPassEncoderSetIndexBuffer(renderPass, state.indexBuffer, WGPUIndexFormat_Uint16, 0, sizeof(indexData));

    // Draw to the screen
    wgpuRenderPassEncoderSetBindGroup(renderPass, 0, state.bindGroup, 0, NULL);
    wgpuRenderPassEncoderDraw(renderPass, 6, 1, 0, 0);
    //wgpuRenderPassEncoderDrawIndexed(renderPass, 6, 1, 0, 0, 0);

    wgpuRenderPassEncoderEnd(renderPass);

    // Finally encode and submit the render pass
    WGPUCommandBufferDescriptor commandBufferDescriptor = {};
    commandBufferDescriptor.nextInChain = NULL;
    commandBufferDescriptor.label = "Command buffer";
    WGPUCommandBuffer command = wgpuCommandEncoderFinish(encoder, &commandBufferDescriptor);

    wgpuQueueSubmit(state.queue, 1, &command);

    // At the end of the frame
    wgpuSurfacePresent(state.surface);

    wgpuTextureViewRelease(targetView);
    wgpuCommandBufferRelease(command);
    wgpuCommandEncoderRelease(encoder);
    wgpuRenderPassEncoderRelease(renderPass);

    wgpuDevicePoll(state.device, false, NULL);
}
