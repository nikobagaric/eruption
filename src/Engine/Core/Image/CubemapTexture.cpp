#include "Engine/Core/Image/CubemapTexture.hpp"
#include "Engine/Core/Buffer/StagingBuffer.hpp"

#include "ext/stb_image.h"

#include <stdexcept>

namespace Engine::Core::Image {

namespace {
constexpr VkFormat kCubemapFormat = VK_FORMAT_R8G8B8A8_SRGB;
constexpr uint32_t kFaceCount = 6;
} // namespace

void CubemapTexture::loadFromFiles(
    Device::PhysicalDevice &physicalDevice,
    const std::array<std::string, 6> &facePaths) {
  int width = 0, height = 0, channels = 0;
  std::array<stbi_uc *, kFaceCount> facePixels{};

  for (uint32_t i = 0; i < kFaceCount; ++i) {
    int faceWidth = 0, faceHeight = 0;
    facePixels[i] = stbi_load(facePaths[i].c_str(), &faceWidth, &faceHeight,
                              &channels, STBI_rgb_alpha);
    if (!facePixels[i]) {
      for (uint32_t j = 0; j < i; ++j) {
        stbi_image_free(facePixels[j]);
      }
      throw std::runtime_error("failed to load cubemap face: " +
                               facePaths[i]);
    }

    if (i == 0) {
      width = faceWidth;
      height = faceHeight;
    } else if (faceWidth != width || faceHeight != height) {
      for (uint32_t j = 0; j <= i; ++j) {
        stbi_image_free(facePixels[j]);
      }
      throw std::runtime_error(
          "cubemap faces must all share the same dimensions: " +
          facePaths[i]);
    }
  }

  const VkDeviceSize faceSize =
      static_cast<VkDeviceSize>(width) * static_cast<VkDeviceSize>(height) * 4;
  const VkDeviceSize totalSize = faceSize * kFaceCount;

  // Vulkan's fixed cube array-layer order is +X, -X, +Y, -Y, +Z, -Z, so the
  // staging buffer packs the faces contiguously in that same order; a single
  // copyBufferToImage call with layerCount=6 then fans it out to all layers.
  Buffer::StagingBuffer stagingBuffer{mDevice, totalSize};
  for (uint32_t i = 0; i < kFaceCount; ++i) {
    stagingBuffer.upload(facePixels[i], faceSize, faceSize * i);
    stbi_image_free(facePixels[i]);
  }

  mImage = std::make_unique<Image>(
      mDevice, static_cast<uint32_t>(width), static_cast<uint32_t>(height),
      kCubemapFormat, VK_IMAGE_TILING_OPTIMAL,
      VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, /*mipLevels=*/1,
      VK_SAMPLE_COUNT_1_BIT, kFaceCount, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT);

  mUploadContext.transitionImageLayout(
      mImage->getImage(), kCubemapFormat, VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, /*mipLevels=*/1, kFaceCount);

  mUploadContext.copyBufferToImage(
      stagingBuffer.getBuffer(), mImage->getImage(),
      static_cast<uint32_t>(width), static_cast<uint32_t>(height),
      kFaceCount);

  mUploadContext.transitionImageLayout(
      mImage->getImage(), kCubemapFormat, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, /*mipLevels=*/1, kFaceCount);

  mImageView = std::make_unique<ImageView>(
      mDevice, mImage->getImage(), kCubemapFormat, VK_IMAGE_ASPECT_COLOR_BIT,
      /*mipLevels=*/1, kFaceCount, VK_IMAGE_VIEW_TYPE_CUBE);

  // Cube faces are sampled by direction near their own edges, so clamping
  // (rather than repeating) avoids bleeding in a neighboring face's texels.
  mSampler = std::make_unique<Sampler>(mDevice, physicalDevice,
                                       /*mipLevels=*/1,
                                       VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
}

CubemapTexture::CubemapTexture(
    Device::Device &device, Device::PhysicalDevice &physicalDevice,
    UploadContext &uploadContext,
    const std::array<std::string, 6> &facePaths)
    : mDevice(device), mUploadContext(uploadContext) {
  loadFromFiles(physicalDevice, facePaths);
}

} // namespace Engine::Core::Image
