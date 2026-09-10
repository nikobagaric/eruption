#include "Engine/Core/Image/Image.hpp"
#include <stdexcept>

namespace Engine::Core::Image {
/////////////////////
// PRIVATE METHODS //
/////////////////////

void Image::createImage(VkImageTiling tiling, VkImageUsageFlags usage,
                        VkMemoryPropertyFlags properties,
                        VkSampleCountFlagBits sampleCount) {
  VkImageCreateInfo imageInfo{};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.extent.width = mWidth;
  imageInfo.extent.height = mHeight;
  imageInfo.extent.depth = 1;
  imageInfo.mipLevels = mMipLevels;
  imageInfo.arrayLayers = 1;
  imageInfo.format = mFormat;
  imageInfo.tiling = tiling;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.usage = usage;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  imageInfo.samples = sampleCount;

  if (vkCreateImage(mDevice.getDevice(), &imageInfo, nullptr, &mImage) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create image!");
  }

  VkMemoryRequirements memRequirements;
  vkGetImageMemoryRequirements(mDevice.getDevice(), mImage, &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex =
      mDevice.findMemoryType(memRequirements.memoryTypeBits, properties);

  if (vkAllocateMemory(mDevice.getDevice(), &allocInfo, nullptr,
                       &mImageMemory) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate image memory!");
  }

  vkBindImageMemory(mDevice.getDevice(), mImage, mImageMemory, 0);
}

////////////////////
// PUBLIC METHODS //
////////////////////

Image::Image(Device::Device &device, uint32_t width, uint32_t height,
             VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
             VkMemoryPropertyFlags properties, uint32_t mipLevels,
             VkSampleCountFlagBits sampleCount)
    : mDevice(device), mFormat(format), mWidth(width), mHeight(height),
      mMipLevels(mipLevels) {
  createImage(tiling, usage, properties, sampleCount);
}

Image::~Image() {
  if (mImage != VK_NULL_HANDLE) {
    vkDestroyImage(mDevice.getDevice(), mImage, nullptr);
    mImage = VK_NULL_HANDLE;
  }
  if (mImageMemory != VK_NULL_HANDLE) {
    vkFreeMemory(mDevice.getDevice(), mImageMemory, nullptr);
    mImageMemory = VK_NULL_HANDLE;
  }
}

} // namespace Engine::Core::Image
