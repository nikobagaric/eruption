#pragma once

#include "Engine/Core/Device/Device.hpp"
#include "Engine/Core/Device/PhysicalDevice.hpp"

#include <vulkan/vulkan.h>

namespace Engine::Core::Commands {
class CommandPool {
public:
  CommandPool(Device::Device &device, Device::PhysicalDevice &physicalDevice);
  ~CommandPool();

  CommandPool(const CommandPool &) = delete;
  CommandPool &operator=(const CommandPool &) = delete;

  CommandPool(CommandPool &&) noexcept = default;
  CommandPool &operator=(CommandPool &&) noexcept = default;

  VkCommandPool getCommandPool() const { return mCommandPool; }

private:
  void createCommandPool();

  Device::Device &mDevice;
  Device::PhysicalDevice &mPhysicalDevice;

  VkCommandPool mCommandPool{VK_NULL_HANDLE};
};
} // namespace Engine::Core::Commands
