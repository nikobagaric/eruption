#pragma once

#include "Engine/Core/Instance/Window.hpp"
#include "Engine/Core/Instance/Instance.hpp"
#include "Engine/Core/Device/PhysicalDevice.hpp"
#include "Engine/Core/Device/Device.hpp"
#include "Engine/Core/Device/SwapChain.hpp"
#include "Engine/Core/Pipeline/Shader.hpp"
#include "Engine/Core/Pipeline/GraphicsPipeline.hpp"
#include "Engine/Core/Pipeline/Framebuffer.hpp"
#include "Engine/Core/Commands/CommandPool.hpp"
#include "Engine/Core/Commands/CommandBuffer.hpp"
#include "Engine/Core/Sync/Semaphore.hpp"
#include "Engine/Core/Sync/Fence.hpp"

#include <memory>
#include <vector>

namespace Engine
{
    class Engine
    {
    public:
        explicit Engine(uint16_t width = 1280, uint16_t height = 720, const std::string &title = "Eruption");
        ~Engine();

        Engine(const Engine &) = delete;
        Engine &operator=(const Engine &) = delete;

        void run();

    private:
        void init();
        void createSyncObjects();
        void createFramebuffers();
        void recordCommandBuffers();
        void drawFrame();

        Core::Instance::Window mWindow;
        std::unique_ptr<Core::Instance::Instance> mInstance;
        std::unique_ptr<Core::Device::PhysicalDevice> mPhysicalDevice;
        std::unique_ptr<Core::Device::Device> mDevice;
        std::unique_ptr<Core::Device::SwapChain> mSwapChain;
        std::unique_ptr<Core::Pipeline::Shader> mVertexShader;
        std::unique_ptr<Core::Pipeline::Shader> mFragmentShader;
        std::unique_ptr<Core::Pipeline::GraphicsPipeline> mGraphicsPipeline;
        std::unique_ptr<Core::Pipeline::Framebuffer> mFramebuffer;
        std::unique_ptr<Core::Commands::CommandPool> mCommandPool;
        std::unique_ptr<Core::Commands::CommandBuffer> mCommandBuffer;
        std::unique_ptr<Core::Sync::SemaphorePool> mSemaphorePool;
        std::unique_ptr<Core::Sync::FencePool> mFencePool;

        size_t mCurrentFrame = 0;
    };
}
