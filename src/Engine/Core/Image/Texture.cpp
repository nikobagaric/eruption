#include "Engine/Core/Image/Texture.hpp"
#include "Engine/Core/Buffer/StagingBuffer.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "ext/stb_image.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace Engine::Core::Image {

namespace {
constexpr VkFormat kTextureFormat = VK_FORMAT_R8G8B8A8_SRGB;
}

void Texture::loadFromFile(Device::PhysicalDevice &physicalDevice,
                           const std::string &path) {
  int width = 0, height = 0, channels = 0;
  stbi_uc *pixels =
      stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);

  if (!pixels) {
    throw std::runtime_error("failed to load texture image: " + path);
  }

  VkDeviceSize imageSize =
      static_cast<VkDeviceSize>(width) * static_cast<VkDeviceSize>(height) * 4;

  uint32_t mipLevels = static_cast<uint32_t>(std::floor(std::log2(
                            std::max(width, height)))) +
                        1;

  Buffer::StagingBuffer stagingBuffer{mDevice, imageSize};
  stagingBuffer.upload(pixels, imageSize);
  stbi_image_free(pixels);

  mImage = std::make_unique<Image>(
      mDevice, static_cast<uint32_t>(width), static_cast<uint32_t>(height),
      kTextureFormat, VK_IMAGE_TILING_OPTIMAL,
      VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
          VK_IMAGE_USAGE_SAMPLED_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mipLevels);

  mUploadContext.transitionImageLayout(
      mImage->getImage(), kTextureFormat, VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);

  mUploadContext.copyBufferToImage(stagingBuffer.getBuffer(),
                                   mImage->getImage(),
                                   static_cast<uint32_t>(width),
                                   static_cast<uint32_t>(height));

  mUploadContext.generateMipmaps(mImage->getImage(), kTextureFormat,
                                 physicalDevice, width, height, mipLevels);

  mImageView = std::make_unique<ImageView>(
      mDevice, mImage->getImage(), kTextureFormat, VK_IMAGE_ASPECT_COLOR_BIT,
      mipLevels);
  mSampler = std::make_unique<Sampler>(mDevice, physicalDevice, mipLevels);
}

Texture::Texture(Device::Device &device, Device::PhysicalDevice &physicalDevice,
                 UploadContext &uploadContext, const std::string &path)
    : mDevice(device), mUploadContext(uploadContext) {
  loadFromFile(physicalDevice, path);
}

} // namespace Engine::Core::Image
