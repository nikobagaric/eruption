#pragma once

#include <vulkan/vulkan.h>

#include "Engine/Core/Device/PhysicalDevice.hpp"

#include <memory>

namespace Engine::Core::Device
{
    class Device
    {
    public:
        explicit Device(PhysicalDevice &physical);
        ~Device();

        Device(const Device &) = delete;
        Device &operator=(const Device &) = delete;

        VkDevice getDevice() const { return mDevice; }
        VkQueue getGraphicsQueue() const { return mGraphicsQueue; }
        VkQueue getPresentQueue() const { return mPresentQueue; }

    private:
        void createLogicalDevice();

        PhysicalDevice &mPhysical;
        VkDevice mDevice{VK_NULL_HANDLE};
        VkQueue mGraphicsQueue{VK_NULL_HANDLE};
        VkQueue mPresentQueue{VK_NULL_HANDLE};
    };

} // namespace Engine::Core::Device
