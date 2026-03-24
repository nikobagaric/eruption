#include "Engine/Core/Device/SwapChain.hpp"

#include <algorithm>
#include <stdexcept>

namespace Engine::Core::Device
{   
    ////////////////////
    // PUBLIC METHODS //
    ////////////////////

    SwapChain::SwapChain(Device &device, PhysicalDevice &physicalDevice, Instance::Instance &instance, Instance::Window &window)
        : mDevice(device)
        , mPhysicalDevice(physicalDevice)
        , mInstance(instance)
        , mWindow(window)
    {
        createSwapChain();
    }

    SwapChain::~SwapChain()
    {
        cleanupSwapChain();
    }

    /////////////////////
    // PRIVATE METHODS //
    /////////////////////

    static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
    {
        for (const auto &availableFormat : availableFormats)
        {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                return availableFormat;
            }
        }

        return availableFormats.empty() ? VkSurfaceFormatKHR{} : availableFormats[0];
    }

    static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
    {
        for (const auto &availablePresentMode : availablePresentModes)
        {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                return availablePresentMode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    static VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities, uint32_t width, uint32_t height)
    {
        if (capabilities.currentExtent.width != UINT32_MAX)
        {
            return capabilities.currentExtent;
        }

        VkExtent2D actualExtent = {width, height};

        actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

        return actualExtent;
    }

    VkImageView SwapChain::createImageView(VkImage image)
    {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = mImageFormat;
        viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VkImageView imageView;
        if (vkCreateImageView(mDevice.getDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create image views!");
        }

        return imageView;
    }

    void SwapChain::createSwapChain()
    {
        auto swapChainSupport = mPhysicalDevice.querySwapChainSupport(mPhysicalDevice.getDevice());

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);

        // prefer current extent, else window size
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities, mWindow.getWidth(), mWindow.getHeight());

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
        {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = mInstance.getSurface();

        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = mPhysicalDevice.findQueueFamilies(mPhysicalDevice.getDevice());
        uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

        if (indices.graphicsFamily != indices.presentFamily)
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices = nullptr;
        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        if (vkCreateSwapchainKHR(mDevice.getDevice(), &createInfo, nullptr, &mSwapChain) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create swap chain!");
        }

        vkGetSwapchainImagesKHR(mDevice.getDevice(), mSwapChain, &imageCount, nullptr);
        mSwapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(mDevice.getDevice(), mSwapChain, &imageCount, mSwapChainImages.data());

        mImageFormat = surfaceFormat.format;
        mExtent = extent;

        mSwapChainImageViews.resize(mSwapChainImages.size());
        for (size_t i = 0; i < mSwapChainImages.size(); i++)
        {
            mSwapChainImageViews[i] = createImageView(mSwapChainImages[i]);
        }
    }

    void SwapChain::cleanupSwapChain()
    {
        for (auto imageView : mSwapChainImageViews)
        {
            vkDestroyImageView(mDevice.getDevice(), imageView, nullptr);
        }

        if (mSwapChain != VK_NULL_HANDLE)
        {
            vkDestroySwapchainKHR(mDevice.getDevice(), mSwapChain, nullptr);
            mSwapChain = VK_NULL_HANDLE;
        }

        mSwapChainImageViews.clear();
        mSwapChainImages.clear();
    }

    void SwapChain::recreateSwapChain()
    {
        cleanupSwapChain();
        createSwapChain();
    }

} // namespace Engine::Core::Device
