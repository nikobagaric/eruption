#include "Engine/Core/Image/ImageView.hpp"
#include <stdexcept>

namespace Engine::Core::Image {

void ImageView::createImageView(VkImage image, VkFormat format,
                                VkImageAspectFlags aspectFlags,
                                uint32_t mipLevels, uint32_t layerCount,
                                VkImageViewType viewType) {
  VkImageViewCreateInfo viewInfo{};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image = image;
  viewInfo.viewType = viewType;
  viewInfo.format = format;
  viewInfo.subresourceRange.aspectMask = aspectFlags;
  viewInfo.subresourceRange.baseMipLevel = 0;
  viewInfo.subresourceRange.levelCount = mipLevels;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount = layerCount;

  if (vkCreateImageView(mDevice.getDevice(), &viewInfo, nullptr,
                        &mImageView) != VK_SUCCESS) {
    throw std::runtime_error("failed to create image view!");
  }
}

ImageView::ImageView(Device::Device &device, VkImage image, VkFormat format,
                     VkImageAspectFlags aspectFlags, uint32_t mipLevels,
                     uint32_t layerCount, VkImageViewType viewType)
    : mDevice(device) {
  createImageView(image, format, aspectFlags, mipLevels, layerCount, viewType);
}

ImageView::~ImageView() {
  if (mImageView != VK_NULL_HANDLE) {
    vkDestroyImageView(mDevice.getDevice(), mImageView, nullptr);
    mImageView = VK_NULL_HANDLE;
  }
}

} // namespace Engine::Core::Image
