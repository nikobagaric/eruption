#include "Engine/Core/Device/PhysicalDevice.hpp"

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

    bool PhysicalDevice::isDeviceSuitable(VkPhysicalDevice device)
    {
        return true;
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

            if (indices.isComplete())
            {
                break;
            }

            i++;
        }

        return indices;
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
