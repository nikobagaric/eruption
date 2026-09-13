#include "Engine/Core/Image/Sampler.hpp"
#include <stdexcept>

namespace Engine::Core::Image {

void Sampler::createSampler(Device::PhysicalDevice &physicalDevice,
                            uint32_t mipLevels,
                            VkSamplerAddressMode addressMode) {
  VkPhysicalDeviceProperties properties{};
  vkGetPhysicalDeviceProperties(physicalDevice.getDevice(), &properties);

  VkSamplerCreateInfo samplerInfo{};
  samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  samplerInfo.magFilter = VK_FILTER_LINEAR;
  samplerInfo.minFilter = VK_FILTER_LINEAR;
  samplerInfo.addressModeU = addressMode;
  samplerInfo.addressModeV = addressMode;
  samplerInfo.addressModeW = addressMode;
  samplerInfo.anisotropyEnable = VK_TRUE;
  samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
  samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  samplerInfo.unnormalizedCoordinates = VK_FALSE;
  samplerInfo.compareEnable = VK_FALSE;
  samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
  samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  samplerInfo.mipLodBias = 0.0f;
  samplerInfo.minLod = 0.0f;
  samplerInfo.maxLod = static_cast<float>(mipLevels);

  if (vkCreateSampler(mDevice.getDevice(), &samplerInfo, nullptr,
                      &mSampler) != VK_SUCCESS) {
    throw std::runtime_error("failed to create texture sampler!");
  }
}

Sampler::Sampler(Device::Device &device, Device::PhysicalDevice &physicalDevice,
                 uint32_t mipLevels, VkSamplerAddressMode addressMode)
    : mDevice(device) {
  createSampler(physicalDevice, mipLevels, addressMode);
}

Sampler::~Sampler() {
  if (mSampler != VK_NULL_HANDLE) {
    vkDestroySampler(mDevice.getDevice(), mSampler, nullptr);
    mSampler = VK_NULL_HANDLE;
  }
}

} // namespace Engine::Core::Image
