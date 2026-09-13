#version 450

layout(set = 0, binding = 0) uniform GlobalUBO {
  mat4 view;
  mat4 proj;
  vec4 lightPos;
  vec4 cameraPos;
  float time;
} ubo;

layout(push_constant) uniform Push {
  mat4 model;
  layout(offset = 68) float uvScale;
} push;

layout(location = 0) in vec3 inPosition;
// Unused here, but must stay declared: the shared vertex input layout
// (Buffer::Vertex) always binds a color attribute at location 1, and
// validation layers flag any bound attribute the vertex shader doesn't
// consume.
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;

layout(location = 0) out vec3 fragNormalWorld;
layout(location = 1) out vec3 fragPosWorld;
layout(location = 2) out vec2 fragTexCoord;

void main() {
  vec4 posWorld = push.model * vec4(inPosition, 1.0);
  gl_Position = ubo.proj * ubo.view * posWorld;

  // mat3(push.model) assumes uniform scale; a non-uniform scale would need
  // the inverse-transpose normal matrix instead.
  fragNormalWorld = normalize(mat3(push.model) * inNormal);
  fragPosWorld = posWorld.xyz;
  fragTexCoord = inTexCoord * push.uvScale;
}
