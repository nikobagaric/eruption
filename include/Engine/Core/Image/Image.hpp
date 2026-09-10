#pragma once

#include <vulkan/vulkan.hpp>

#include "Engine/Core/Device/Device.hpp"

namespace Engine::Core::Image {
class Image {
public:
  Image(Device::Device &device, uint32_t width, uint32_t height,
        VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
        VkMemoryPropertyFlags properties, uint32_t mipLevels = 1,
        VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT);
  ~Image();

  Image(const Image &) = delete;
  Image &operator=(const Image &) = delete;

  Image(Image &&) noexcept = default;
  Image &operator=(Image &&) noexcept = delete;

  VkImage getImage() const { return mImage; }
  VkDeviceMemory getMemory() const { return mImageMemory; }
  VkFormat getFormat() const { return mFormat; }
  uint32_t getWidth() const { return mWidth; }
  uint32_t getHeight() const { return mHeight; }
  uint32_t getMipLevels() const { return mMipLevels; }

private:
  Device::Device &mDevice;

  VkImage mImage{VK_NULL_HANDLE};
  VkDeviceMemory mImageMemory{VK_NULL_HANDLE};
  VkFormat mFormat;
  uint32_t mWidth;
  uint32_t mHeight;
  uint32_t mMipLevels;

  void createImage(VkImageTiling tiling, VkImageUsageFlags usage,
                   VkMemoryPropertyFlags properties,
                   VkSampleCountFlagBits sampleCount);
};
} // namespace Engine::Core::Image
