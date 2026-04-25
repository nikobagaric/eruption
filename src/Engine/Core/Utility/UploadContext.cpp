#include "Engine/Core/Utility/UploadContext.hpp"
#include "Engine/Core/Commands/CommandBuffer.hpp"
#include "Engine/Core/Sync/Fence.hpp"

namespace Engine::Core {

void UploadContext::copyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size,
                               VkDeviceSize srcOffset, VkDeviceSize dstOffset) {
  Commands::CommandBuffer cmd{device, cmdPool, 1};
  Sync::Fence fence{device};

  VkCommandBuffer rawCmd = cmd.getCommandBuffer();

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(rawCmd, &beginInfo);

  VkBufferCopy copyRegion{};
  copyRegion.srcOffset = srcOffset;
  copyRegion.dstOffset = dstOffset;
  copyRegion.size = size;

  vkCmdCopyBuffer(rawCmd, src, dst, 1, &copyRegion);

  vkEndCommandBuffer(rawCmd);

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &rawCmd;

  vkQueueSubmit(transferQueue, 1, &submitInfo, fence.getFence());

  fence.wait();
}

} // namespace Engine::Core
