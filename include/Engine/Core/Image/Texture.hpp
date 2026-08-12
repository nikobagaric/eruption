#pragma once

#include <memory>
#include <string>

#include "Engine/Core/Device/Device.hpp"
#include "Engine/Core/Device/PhysicalDevice.hpp"
#include "Engine/Core/Image/Image.hpp"
#include "Engine/Core/Image/ImageView.hpp"
#include "Engine/Core/Image/Sampler.hpp"
#include "Engine/Core/Utility/UploadContext.hpp"

namespace Engine::Core::Image {

inline constexpr uint32_t kMaxTextures = 32;

class Texture {
public:
  Texture(Device::Device &device, Device::PhysicalDevice &physicalDevice,
         UploadContext &uploadContext, const std::string &path);

  Texture(const Texture &) = delete;
  Texture &operator=(const Texture &) = delete;

  Texture(Texture &&) noexcept = default;
  Texture &operator=(Texture &&) noexcept = delete;

  VkImageView getImageView() const { return mImageView->getImageView(); }
  VkSampler getSampler() const { return mSampler->getSampler(); }

  VkDescriptorImageInfo getDescriptorImageInfo() const {
    return VkDescriptorImageInfo{
        .sampler = getSampler(),
        .imageView = getImageView(),
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };
  }

private:
  Device::Device &mDevice;
  UploadContext &mUploadContext;

  std::unique_ptr<Image> mImage;
  std::unique_ptr<ImageView> mImageView;
  std::unique_ptr<Sampler> mSampler;

  void loadFromFile(Device::PhysicalDevice &physicalDevice,
                    const std::string &path);
};

} // namespace Engine::Core::Image
