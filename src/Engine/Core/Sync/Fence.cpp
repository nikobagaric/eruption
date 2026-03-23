#include "Engine/Core/Sync/Fence.hpp"

#include <stdexcept>

namespace Engine::Core::Sync
{
    ////////////////////
    // FENCE METHODS  //
    ////////////////////

    /////////////////////
    // PRIVATE METHODS //
    /////////////////////

    void Fence::createFence(bool signaled)
    {
        VkFenceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        createInfo.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

        if (vkCreateFence(mDevice.getDevice(), &createInfo, nullptr, &mFence) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create fence!");
        }
    }

    ////////////////////
    // PUBLIC METHODS //
    ////////////////////

    Fence::Fence(Device::Device& device, bool signaled)
        : mDevice(device)
    {
        createFence(signaled);
    }

    Fence::~Fence()
    {
        if (mFence != VK_NULL_HANDLE)
        {
            vkDestroyFence(mDevice.getDevice(), mFence, nullptr);
            mFence = VK_NULL_HANDLE;
        }
    }

    void Fence::wait(uint64_t timeout) const
    {
        vkWaitForFences(mDevice.getDevice(), 1, &mFence, VK_TRUE, timeout);
    }

    void Fence::reset() const
    {
        vkResetFences(mDevice.getDevice(), 1, &mFence);
    }

    ///////////////////////
    // FENCE POOL METHODS //
    ///////////////////////

    /////////////////////
    // PRIVATE METHODS //
    /////////////////////

    void FencePool::createFences()
    {
        mFences.resize(mPoolSize);

        VkFenceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        createInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (uint32_t i = 0; i < mPoolSize; ++i)
        {
            if (vkCreateFence(mDevice.getDevice(), &createInfo, nullptr, &mFences[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create fence pool!");
            }
        }
    }

    ////////////////////
    // PUBLIC METHODS //
    ////////////////////

    FencePool::FencePool(Device::Device& device, uint32_t poolSize)
        : mDevice(device), mPoolSize(poolSize)
    {
        createFences();
    }

    FencePool::~FencePool()
    {
        for (VkFence fence : mFences)
        {
            if (fence != VK_NULL_HANDLE)
            {
                vkDestroyFence(mDevice.getDevice(), fence, nullptr);
            }
        }
        mFences.clear();
    }

} // namespace Engine::Core::Sync
