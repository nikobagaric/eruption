#pragma once

#include <vulkan/vulkan.hpp>

#include "Engine/Core/Device/Device.hpp"

namespace Engine::Core::Image {
class ImageView {
public:
  ImageView(Device::Device &device, VkImage image, VkFormat format,
           VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT);
  ~ImageView();

  ImageView(const ImageView &) = delete;
  ImageView &operator=(const ImageView &) = delete;

  ImageView(ImageView &&) noexcept = default;
  ImageView &operator=(ImageView &&) noexcept = delete;

  VkImageView getImageView() const { return mImageView; }

private:
  Device::Device &mDevice;
  VkImageView mImageView{VK_NULL_HANDLE};

  void createImageView(VkImage image, VkFormat format,
                       VkImageAspectFlags aspectFlags);
};
} // namespace Engine::Core::Image
