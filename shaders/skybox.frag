#version 450

layout(set = 0, binding = 2) uniform samplerCube skyboxSampler;

layout(location = 0) in vec3 fragDirection;

layout(location = 0) out vec4 outColor;

void main() {
  outColor = texture(skyboxSampler, fragDirection);
}
