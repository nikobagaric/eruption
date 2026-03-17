#pragma once

#include <vulkan/vulkan.h>

#include <optional>
#include <vector>

#include "Engine/Core/Instance/Window.hpp"
#include "Engine/Core/Instance/Instance.hpp"

namespace Engine::Core::Device
{

    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete()
        {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    class PhysicalDevice
    {
    public:
        explicit PhysicalDevice(Instance::Instance &instance, Instance::Window &window);
        ~PhysicalDevice();

        PhysicalDevice(const PhysicalDevice &) = delete;
        PhysicalDevice &operator=(const PhysicalDevice &) = delete;

        VkPhysicalDevice getDevice() const { return mDevice; }

        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) const;

    private:
        void pickPhysicalDevice();
        bool isDeviceSuitable(VkPhysicalDevice device);

        VkPhysicalDevice mDevice{VK_NULL_HANDLE};
        Instance::Instance &mInstance;
        Instance::Window &mWindow;
    };
}