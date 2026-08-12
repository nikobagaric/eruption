#include "Engine/Core/Device/PhysicalDevice.hpp"

#include <set>
#include <string>

namespace Engine::Core::Device
{
    /////////////////////
    // PRIVATE METHODS //
    /////////////////////

    void PhysicalDevice::pickPhysicalDevice()
    {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(mInstance.getInstance(), &deviceCount, nullptr);
        if (deviceCount == 0)
        {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(mInstance.getInstance(), &deviceCount, devices.data());

        for (const auto &dev : devices)
        {
            if (isDeviceSuitable(dev))
            {
                mDevice = dev;
                break;
            }
        }

        if (mDevice == VK_NULL_HANDLE)
        {
            throw std::runtime_error("failed to find a suitable GPU!");
        }
    }

    bool PhysicalDevice::isDeviceSuitable(VkPhysicalDevice device) const
    {
        QueueFamilyIndices indices = findQueueFamilies(device);

        bool extensionsSupported = checkDeviceExtensionSupport(device);

        bool swapChainAdequate = false;
        if (extensionsSupported)
        {
            auto swapChainSupport = querySwapChainSupport(device);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

        return indices.isComplete() && extensionsSupported && swapChainAdequate &&
               supportedFeatures.samplerAnisotropy &&
               supportedFeatures.shaderSampledImageArrayDynamicIndexing;
    }

    QueueFamilyIndices PhysicalDevice::findQueueFamilies(VkPhysicalDevice device) const
    {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto &queueFamily : queueFamilies)
        {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                indices.graphicsFamily = i;
            }

            VkBool32 presentSupport = VK_FALSE;
            if (mInstance.getSurface() != VK_NULL_HANDLE)
            {
                vkGetPhysicalDeviceSurfaceSupportKHR(device, i, mInstance.getSurface(), &presentSupport);
                if (presentSupport)
                {
                    indices.presentFamily = i;
                }
            }

            if (indices.isComplete())
            {
                break;
            }

            i++;
        }

        return indices;
    }

    bool PhysicalDevice::checkDeviceExtensionSupport(VkPhysicalDevice device) const
    {
        const std::vector<const char *> deviceExtensions = {"VK_KHR_swapchain"};

        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto &extension : availableExtensions)
        {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    PhysicalDevice::SwapChainSupportDetails PhysicalDevice::querySwapChainSupport(VkPhysicalDevice device) const
    {
        SwapChainSupportDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, mInstance.getSurface(), &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, mInstance.getSurface(), &formatCount, nullptr);

        if (formatCount != 0)
        {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, mInstance.getSurface(), &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, mInstance.getSurface(), &presentModeCount, nullptr);

        if (presentModeCount != 0)
        {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, mInstance.getSurface(), &presentModeCount, details.presentModes.data());
        }

        return details;
    }

    ////////////////////
    // PUBLIC METHODS //
    ////////////////////

    PhysicalDevice::PhysicalDevice(Instance::Instance &instance, Instance::Window &window) : mInstance(instance), mWindow(window)
    {
        pickPhysicalDevice();
    }

    PhysicalDevice::~PhysicalDevice()
    {
    }

} // namespace Engine::Core::Device
