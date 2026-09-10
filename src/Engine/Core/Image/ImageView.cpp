#include "Engine/Core/Image/ImageView.hpp"
#include <stdexcept>

namespace Engine::Core::Image {

void ImageView::createImageView(VkImage image, VkFormat format,
                                VkImageAspectFlags aspectFlags,
                                uint32_t mipLevels) {
  VkImageViewCreateInfo viewInfo{};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image = image;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format = format;
  viewInfo.subresourceRange.aspectMask = aspectFlags;
  viewInfo.subresourceRange.baseMipLevel = 0;
  viewInfo.subresourceRange.levelCount = mipLevels;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount = 1;

  if (vkCreateImageView(mDevice.getDevice(), &viewInfo, nullptr,
                        &mImageView) != VK_SUCCESS) {
    throw std::runtime_error("failed to create image view!");
  }
}

ImageView::ImageView(Device::Device &device, VkImage image, VkFormat format,
                     VkImageAspectFlags aspectFlags, uint32_t mipLevels)
    : mDevice(device) {
  createImageView(image, format, aspectFlags, mipLevels);
}

ImageView::~ImageView() {
  if (mImageView != VK_NULL_HANDLE) {
    vkDestroyImageView(mDevice.getDevice(), mImageView, nullptr);
    mImageView = VK_NULL_HANDLE;
  }
}

} // namespace Engine::Core::Image
