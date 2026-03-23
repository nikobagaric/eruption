#pragma once

#include "Engine/Core/Device/Device.hpp"

#include <vulkan/vulkan.h>
#include <vector>

namespace Engine::Core::Sync
{
    class Fence {
    public:
        explicit Fence(Device::Device& device, bool signaled = false);
        ~Fence();

        Fence(const Fence&) = delete;
        Fence& operator=(const Fence&) = delete;

        Fence(Fence&&) noexcept = default;
        Fence& operator=(Fence&&) noexcept = default;

        VkFence getFence() const { return mFence; }

        void wait(uint64_t timeout = UINT64_MAX) const;
        void reset() const;

    private:
        void createFence(bool signaled);

        Device::Device& mDevice;
        VkFence mFence{VK_NULL_HANDLE};
    };

    class FencePool {
    public:
        explicit FencePool(Device::Device& device, uint32_t poolSize = 1);
        ~FencePool();

        FencePool(const FencePool&) = delete;
        FencePool& operator=(const FencePool&) = delete;

        FencePool(FencePool&&) noexcept = default;
        FencePool& operator=(FencePool&&) noexcept = default;

        const std::vector<VkFence>& getFences() const { return mFences; }
        VkFence getFence(uint32_t index = 0) const { return mFences[index]; }

    private:
        void createFences();

        Device::Device& mDevice;
        uint32_t mPoolSize;
        std::vector<VkFence> mFences;
    };
} // namespace Engine::Core::Sync
