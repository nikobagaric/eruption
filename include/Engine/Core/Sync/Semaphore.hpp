#pragma once

#include "Engine/Core/Device/Device.hpp"

#include <vulkan/vulkan.h>
#include <vector>

namespace Engine::Core::Sync
{
    class Semaphore {
    public:
        explicit Semaphore(Device::Device& device);
        ~Semaphore();

        Semaphore(const Semaphore&) = delete;
        Semaphore& operator=(const Semaphore&) = delete;

        Semaphore(Semaphore&&) noexcept = default;
        Semaphore& operator=(Semaphore&&) noexcept = default;

        VkSemaphore getSemaphore() const { return mSemaphore; }

    private:
        void createSemaphore();

        Device::Device& mDevice;
        VkSemaphore mSemaphore{VK_NULL_HANDLE};
    };

    class SemaphorePool {
    public:
        explicit SemaphorePool(Device::Device& device, uint32_t poolSize = 2);
        ~SemaphorePool();

        SemaphorePool(const SemaphorePool&) = delete;
        SemaphorePool& operator=(const SemaphorePool&) = delete;

        SemaphorePool(SemaphorePool&&) noexcept = default;
        SemaphorePool& operator=(SemaphorePool&&) noexcept = default;

        const std::vector<VkSemaphore>& getSemaphores() const { return mSemaphores; }
        VkSemaphore getSemaphore(uint32_t index = 0) const { return mSemaphores[index]; }

    private:
        void createSemaphores();

        Device::Device& mDevice;
        uint32_t mPoolSize;
        std::vector<VkSemaphore> mSemaphores;
    };
} // namespace Engine::Core::Sync
