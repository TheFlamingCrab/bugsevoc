// REMOVE THIS LATER THIS IS ONLY FOR TESTING PURPOSES
struct CameraUniform {
    position: vec2f,
    zoom: f32,
}

@group(0) @binding(0) var<uniform> cameraUniform: CameraUniform;

struct VertexInput {
    @location(0) position: vec2f,
    @location(1) color: vec3f,
};

struct VertexOutput {
    @builtin(position) position: vec4f,
    @location(0) color: vec3f,
};

fn worldToScreen(coordinates: vec2f) -> vec2f{
    return coordinates;
}

@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    //let ratio = 640.0 / 480.0;
    //TODO: MOVE THIS CODE INTO A FUNCTION
    out.position = vec4f(((in.position.x - cameraUniform.position.x) * cameraUniform.zoom), ((in.position.y - cameraUniform.position.y) * cameraUniform.zoom), 0.0, 1.0);
    out.color = in.color;
    return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
    return vec4f(in.color, 1.0);
}