#include "Engine/Core/Commands/CommandPool.hpp"

#include <stdexcept>

namespace Engine::Core::Commands
{
    ////////////////////
    // PUBLIC METHODS //
    ////////////////////

    CommandPool::CommandPool(Device::Device& device, Device::PhysicalDevice& physicalDevice)
        : mDevice(device), mPhysicalDevice(physicalDevice)
    {
        createCommandPool();
    }

    CommandPool::~CommandPool()
    {
        if (mCommandPool != VK_NULL_HANDLE)
        {
            vkDestroyCommandPool(mDevice.getDevice(), mCommandPool, nullptr);
            mCommandPool = VK_NULL_HANDLE;
        }
    }

    /////////////////////
    // PRIVATE METHODS //
    /////////////////////

    void CommandPool::createCommandPool()
    {
        Device::QueueFamilyIndices queueFamilyIndices = mPhysicalDevice.findQueueFamilies(mPhysicalDevice.getDevice());

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

        if (vkCreateCommandPool(mDevice.getDevice(), &poolInfo, nullptr, &mCommandPool) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create command pool!");
        }
    }

} // namespace Engine::Core::Commands
