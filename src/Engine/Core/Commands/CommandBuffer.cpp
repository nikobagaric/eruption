#include "Engine/Core/Commands/CommandBuffer.hpp"

#include <stdexcept>

namespace Engine::Core::Commands
{
    ////////////////////
    // PUBLIC METHODS //
    ////////////////////

    CommandBuffer::CommandBuffer(Device::Device& device, CommandPool& commandPool, uint32_t bufferCount)
        : mDevice(device), mCommandPool(commandPool), mBufferCount(bufferCount)
    {
        allocateCommandBuffers();
    }

    CommandBuffer::~CommandBuffer()
    {
        freeCommandBuffers();
    }

    /////////////////////
    // PRIVATE METHODS //
    /////////////////////

    void CommandBuffer::allocateCommandBuffers()
    {
        mCommandBuffers.resize(mBufferCount);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = mCommandPool.getCommandPool();
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = mBufferCount;

        if (vkAllocateCommandBuffers(mDevice.getDevice(), &allocInfo, mCommandBuffers.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }

    void CommandBuffer::freeCommandBuffers()
    {
        if (!mCommandBuffers.empty())
        {
            vkFreeCommandBuffers(mDevice.getDevice(), mCommandPool.getCommandPool(),
                                static_cast<uint32_t>(mCommandBuffers.size()), mCommandBuffers.data());
            mCommandBuffers.clear();
        }
    }

} // namespace Engine::Core::Commands
