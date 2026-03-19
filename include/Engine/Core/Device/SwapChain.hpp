#pragma once

#include <vulkan/vulkan.h>
#include <vector>

#include "Engine/Core/Device/Device.hpp"
#include "Engine/Core/Device/PhysicalDevice.hpp"
#include "Engine/Core/Instance/Window.hpp"
#include "Engine/Core/Instance/Instance.hpp"

namespace Engine::Core::Device
{
    class SwapChain
    {
    public:
        SwapChain(Device &device, PhysicalDevice &physicalDevice, Instance::Instance &instance, Instance::Window &window);
        ~SwapChain();

        SwapChain(const SwapChain &) = delete;
        SwapChain &operator=(const SwapChain &) = delete;

        VkSwapchainKHR getSwapChain() const { return mSwapChain; }
        VkFormat getImageFormat() const { return mImageFormat; }
        VkExtent2D getExtent() const { return mExtent; }
        const std::vector<VkImageView> &getImageViews() const { return mSwapChainImageViews; }

    private:
        void createSwapChain();
        void cleanupSwapChain();

        VkImageView createImageView(VkImage image);

        Device &mDevice;
        PhysicalDevice &mPhysicalDevice;
        Instance::Instance &mInstance;
        Instance::Window &mWindow;

        VkSwapchainKHR mSwapChain{VK_NULL_HANDLE};
        std::vector<VkImage> mSwapChainImages;
        std::vector<VkImageView> mSwapChainImageViews;
        VkFormat mImageFormat{VK_FORMAT_UNDEFINED};
        VkExtent2D mExtent{0, 0};
    };
}
