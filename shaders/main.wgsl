// REMOVE THIS LATER THIS IS ONLY FOR TESTING PURPOSES
struct UniformData {
    cameraPosition: vec2f,
}

@group(0) @binding(0) var<uniform> uniforms: UniformData;

struct VertexInput {
    @location(0) position: vec2f,
    @location(1) color: vec3f,
};

struct VertexOutput {
    @builtin(position) position: vec4f,
    @location(0) color: vec3f,
};

@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    let ratio = 640.0 / 480.0;
    out.position = vec4f(in.position.x + uniforms.cameraPosition.x, in.position.y * ratio + uniforms.cameraPosition.y, 0.0, 1.0);
    out.color = in.color;
    return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
    return vec4f(in.color, 1.0);
}