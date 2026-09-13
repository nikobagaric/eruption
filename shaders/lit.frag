#version 450

layout(set = 0, binding = 0) uniform GlobalUBO {
  mat4 view;
  mat4 proj;
  vec4 lightPos;
  vec4 cameraPos;
  float time;
} ubo;

layout(set = 0, binding = 1) uniform sampler2D textures[32];

layout(push_constant) uniform Push {
  layout(offset = 64) int textureIndex;
} push;

layout(location = 0) in vec3 fragNormalWorld;
layout(location = 1) in vec3 fragPosWorld;
layout(location = 2) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

const float kAmbientStrength = 0.08;
const float kDiffuseStrength = 0.35;
const float kSpecularStrength = 0.9;
const float kShininess = 64.0;

void main() {
  vec3 albedo = texture(textures[push.textureIndex], fragTexCoord).rgb;

  vec3 N = normalize(fragNormalWorld);
  vec3 L = normalize(ubo.lightPos.xyz - fragPosWorld);
  vec3 V = normalize(ubo.cameraPos.xyz - fragPosWorld);
  vec3 H = normalize(L + V);

  float diff = max(dot(N, L), 0.0);
  float spec = pow(max(dot(N, H), 0.0), kShininess);

  // Metal has near-zero diffuse reflectance and a specular highlight tinted
  // by its own color rather than the light's, so both terms are scaled by
  // the sampled albedo instead of a separate material color.
  vec3 ambient = kAmbientStrength * albedo;
  vec3 diffuse = kDiffuseStrength * diff * albedo;
  vec3 specular = kSpecularStrength * spec * albedo;

  outColor = vec4(ambient + diffuse + specular, 1.0);
}
