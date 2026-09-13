#pragma once

#include <array>
#include <memory>
#include <string>

#include "Engine/Core/Device/Device.hpp"
#include "Engine/Core/Device/PhysicalDevice.hpp"
#include "Engine/Core/Image/Image.hpp"
#include "Engine/Core/Image/ImageView.hpp"
#include "Engine/Core/Image/Sampler.hpp"
#include "Engine/Core/Utility/UploadContext.hpp"

namespace Engine::Core::Image {

// A 6-layer cube image sampled with samplerCube, distinct from Texture's
// sampler2D array slots. facePaths must be ordered +X, -X, +Y, -Y, +Z, -Z to
// match Vulkan's fixed cube array-layer order.
class CubemapTexture {
public:
  CubemapTexture(Device::Device &device, Device::PhysicalDevice &physicalDevice,
                 UploadContext &uploadContext,
                 const std::array<std::string, 6> &facePaths);

  CubemapTexture(const CubemapTexture &) = delete;
  CubemapTexture &operator=(const CubemapTexture &) = delete;

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

  void loadFromFiles(Device::PhysicalDevice &physicalDevice,
                     const std::array<std::string, 6> &facePaths);
};

} // namespace Engine::Core::Image
