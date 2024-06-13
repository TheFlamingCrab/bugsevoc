// REMOVE THIS LATER THIS IS ONLY FOR TESTING PURPOSES
struct CameraUniform {
    position: vec2f,
    zoom: f32,
}

@group(0) @binding(0) var<uniform> cameraUniform: CameraUniform;
@group(0) @binding(1) var gradientTexture: texture_2d<f32>;

// TODO: REMOVE COLOR FROM VERTEXINPUT
struct VertexInput {
    @location(0) relative_position: vec2f,
    @location(1) color: vec3f,
    @location(2) instance_position: vec2f,
    @location(3) instance_size: f32,
};

struct VertexOutput {
    @builtin(position) position: vec4f,
    @location(0) uv: vec2f,
};

fn worldToScreen(coordinates: vec2f) -> vec2f{
    return coordinates;
}

@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    //let ratio = 640.0 / 480.0;
    //TODO: MOVE THIS CODE INTO A FUNCTION
    var screenCoordinates: vec2f = vec2f((in.instance_position.x + in.relative_position.x - cameraUniform.position.x) * cameraUniform.zoom, (in.instance_position.y + in.relative_position.y - cameraUniform.position.y) * cameraUniform.zoom);
    out.position = vec4f(screenCoordinates, 0.0, 1.0);
    out.uv = in.relative_position.xy + 0.5;
    return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
    let texelCoords = vec2i(in.uv * vec2f(textureDimensions(gradientTexture)));
    let color = textureLoad(gradientTexture, texelCoords, 0).rgb;
    return vec4f((color), 1.0);
}