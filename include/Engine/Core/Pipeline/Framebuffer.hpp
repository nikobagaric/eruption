#pragma once

#include "Engine/Core/Device/Device.hpp"
#include "Engine/Core/Device/SwapChain.hpp"

#include <vulkan/vulkan.h>
#include <vector>

namespace Engine::Core::Pipeline
{
    class Framebuffer {
    public:
        Framebuffer(Device::Device& device, Device::SwapChain& swapChain, VkRenderPass renderPass);
        ~Framebuffer();

        Framebuffer(const Framebuffer&) = delete;
        Framebuffer& operator=(const Framebuffer&) = delete;

        Framebuffer(Framebuffer&&) noexcept = default;
        Framebuffer& operator=(Framebuffer&&) noexcept = default;

        const std::vector<VkFramebuffer>& getFramebuffers() const { return mFramebuffers; }

    private:
        void createFramebuffers();
        void cleanupFramebuffers();

        Device::Device& mDevice;
        Device::SwapChain& mSwapChain;
        VkRenderPass mRenderPass{VK_NULL_HANDLE};

        std::vector<VkFramebuffer> mFramebuffers;
    };
} // namespace Engine::Core::Pipeline
