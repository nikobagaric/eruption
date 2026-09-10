#pragma once

#include "Engine/Core/Commands/CommandPool.hpp"
#include "Engine/Core/Device/Device.hpp"
#include "Engine/Core/Device/PhysicalDevice.hpp"
#include <vulkan/vulkan.hpp>

namespace Engine::Core {
struct UploadContext {

  Device::Device &device;
  Commands::CommandPool &cmdPool;
  VkQueue transferQueue;

  UploadContext(Device::Device &device, Commands::CommandPool &cmdPool,
                VkQueue transferQueue)
      : device(device), cmdPool(cmdPool), transferQueue(transferQueue) {}

  void copyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size,
                  VkDeviceSize srcOffset = 0, VkDeviceSize dstOffset = 0);

  void copyBufferToImage(VkBuffer src, VkImage dst, uint32_t width,
                         uint32_t height, uint32_t layerCount = 1);

  void transitionImageLayout(VkImage image, VkFormat format,
                             VkImageLayout oldLayout, VkImageLayout newLayout,
                             uint32_t mipLevels = 1);

  void generateMipmaps(VkImage image, VkFormat format,
                       Device::PhysicalDevice &physicalDevice,
                       int32_t texWidth, int32_t texHeight,
                       uint32_t mipLevels);
};
} // namespace Engine::Core
