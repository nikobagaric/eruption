#include "Engine/Core/Utility/UploadContext.hpp"
#include "Engine/Core/Commands/CommandBuffer.hpp"
#include "Engine/Core/Sync/Fence.hpp"

#include <functional>
#include <stdexcept>

namespace Engine::Core {

namespace {
void submitOneTimeCommands(
    Device::Device &device, Commands::CommandPool &cmdPool,
    VkQueue queue, const std::function<void(VkCommandBuffer)> &record) {
  Commands::CommandBuffer cmd{device, cmdPool, 1};
  Sync::Fence fence{device};

  VkCommandBuffer rawCmd = cmd.getCommandBuffer();

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(rawCmd, &beginInfo);

  record(rawCmd);

  vkEndCommandBuffer(rawCmd);

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &rawCmd;

  vkQueueSubmit(queue, 1, &submitInfo, fence.getFence());

  fence.wait();
}
} // namespace

void UploadContext::copyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size,
                               VkDeviceSize srcOffset, VkDeviceSize dstOffset) {
  submitOneTimeCommands(device, cmdPool, transferQueue,
                        [&](VkCommandBuffer rawCmd) {
                          VkBufferCopy copyRegion{};
                          copyRegion.srcOffset = srcOffset;
                          copyRegion.dstOffset = dstOffset;
                          copyRegion.size = size;

                          vkCmdCopyBuffer(rawCmd, src, dst, 1, &copyRegion);
                        });
}

void UploadContext::copyBufferToImage(VkBuffer src, VkImage dst,
                                      uint32_t width, uint32_t height,
                                      uint32_t layerCount) {
  submitOneTimeCommands(device, cmdPool, transferQueue,
                        [&](VkCommandBuffer rawCmd) {
                          VkBufferImageCopy region{};
                          region.bufferOffset = 0;
                          region.bufferRowLength = 0;
                          region.bufferImageHeight = 0;

                          region.imageSubresource.aspectMask =
                              VK_IMAGE_ASPECT_COLOR_BIT;
                          region.imageSubresource.mipLevel = 0;
                          region.imageSubresource.baseArrayLayer = 0;
                          region.imageSubresource.layerCount = layerCount;

                          region.imageOffset = {0, 0, 0};
                          region.imageExtent = {width, height, 1};

                          vkCmdCopyBufferToImage(
                              rawCmd, src, dst,
                              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                              &region);
                        });
}

void UploadContext::transitionImageLayout(VkImage image, VkFormat format,
                                          VkImageLayout oldLayout,
                                          VkImageLayout newLayout) {
  (void)format;

  submitOneTimeCommands(device, cmdPool, transferQueue, [&](VkCommandBuffer
                                                                 rawCmd) {
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
        newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
      barrier.srcAccessMask = 0;
      barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

      sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
      destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
               newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
      barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

      sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
      destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
      throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(rawCmd, sourceStage, destinationStage, 0, 0, nullptr,
                         0, nullptr, 1, &barrier);
  });
}

} // namespace Engine::Core
