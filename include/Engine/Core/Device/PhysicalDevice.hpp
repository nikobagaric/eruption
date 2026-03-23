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
        struct SwapChainSupportDetails
        {
            VkSurfaceCapabilitiesKHR capabilities;
            std::vector<VkSurfaceFormatKHR> formats;
            std::vector<VkPresentModeKHR> presentModes;
        };

        explicit PhysicalDevice(Instance::Instance &instance, Instance::Window &window);
        ~PhysicalDevice();

        PhysicalDevice(const PhysicalDevice &) = delete;
        PhysicalDevice &operator=(const PhysicalDevice &) = delete;

        VkPhysicalDevice getDevice() const { return mDevice; }

        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) const;
        bool isDeviceSuitable(VkPhysicalDevice device) const;
        bool checkDeviceExtensionSupport(VkPhysicalDevice device) const;
        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) const;

        friend class SwapChain;

    private:
        void pickPhysicalDevice();

        VkPhysicalDevice mDevice{VK_NULL_HANDLE};
        Instance::Instance &mInstance;
        Instance::Window &mWindow;
    };
}