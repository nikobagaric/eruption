#pragma once

#include <vulkan/vulkan.h>

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

    private:
        void initVulkan();
        void createInstance();

        VkInstance mVkInstance;
    };
}