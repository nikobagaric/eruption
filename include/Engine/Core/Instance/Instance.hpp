#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <iostream>

namespace Engine::Core::Instance
{
    class Instance
    {
    public:
        explicit Instance();
        ~Instance() = default;

        Instance(const Instance&) = delete;
        Instance& operator=(const Instance&) = delete;  

        // @todo: find out what to do with move op

        VkInstance getInstance() const { return mVkInstance; }
        VkSurfaceKHR getSurface() const { return mSurface; }

    private:
        void createInstance();

        inline void setupDebugMessenger();
        
        std::vector<const char*> getRequiredExtensions();
        bool checkValidationLayerSupport();

        VkInstance mVkInstance;
        VkSurfaceKHR mSurface;
        VkDebugUtilsMessengerEXT mDebugMessenger;

        bool mEnableValidationLayers;
        
        std::vector<const char*> mValidationLayers;
        std::vector<const char*> mRequiredExtensions;
    };
}