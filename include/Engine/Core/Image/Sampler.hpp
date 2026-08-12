#pragma once

#include <vulkan/vulkan.hpp>

#include "Engine/Core/Device/Device.hpp"
#include "Engine/Core/Device/PhysicalDevice.hpp"

namespace Engine::Core::Image {
class Sampler {
public:
  Sampler(Device::Device &device, Device::PhysicalDevice &physicalDevice);
  ~Sampler();

  Sampler(const Sampler &) = delete;
  Sampler &operator=(const Sampler &) = delete;

  Sampler(Sampler &&) noexcept = default;
  Sampler &operator=(Sampler &&) noexcept = delete;

  VkSampler getSampler() const { return mSampler; }

private:
  Device::Device &mDevice;
  VkSampler mSampler{VK_NULL_HANDLE};

  void createSampler(Device::PhysicalDevice &physicalDevice);
};
} // namespace Engine::Core::Image
