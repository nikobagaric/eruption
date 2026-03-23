#pragma once

#include "Engine/Core/Device/Device.hpp"
#include "CommandPool.hpp"

#include <vulkan/vulkan.h>
#include <vector>

namespace Engine::Core::Commands
{
    class CommandBuffer {
    public:
        CommandBuffer(Device::Device& device, CommandPool& commandPool, uint32_t bufferCount = 1);
        ~CommandBuffer();

        CommandBuffer(const CommandBuffer&) = delete;
        CommandBuffer& operator=(const CommandBuffer&) = delete;

        CommandBuffer(CommandBuffer&&) noexcept = default;
        CommandBuffer& operator=(CommandBuffer&&) noexcept = default;

        const std::vector<VkCommandBuffer>& getCommandBuffers() const { return mCommandBuffers; }
        VkCommandBuffer getCommandBuffer(uint32_t index = 0) const { return mCommandBuffers[index]; }

    private:
        void allocateCommandBuffers();
        void freeCommandBuffers();

        Device::Device& mDevice;
        CommandPool& mCommandPool;
        uint32_t mBufferCount;

        std::vector<VkCommandBuffer> mCommandBuffers;
    };
} // namespace Engine::Core::Commands
