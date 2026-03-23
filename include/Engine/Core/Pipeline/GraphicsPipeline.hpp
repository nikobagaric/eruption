#pragma once

#include "Engine/Core/Device/Device.hpp"
#include "Engine/Core/Device/SwapChain.hpp"
#include "Shader.hpp"

#include <vulkan/vulkan.h>

namespace Engine::Core::Pipeline
{
    class GraphicsPipeline {
    public:
        GraphicsPipeline(Device::Device& device, Device::SwapChain& swapChain, Shader& vertexShader, Shader& fragmentShader);
        ~GraphicsPipeline();

        GraphicsPipeline(const GraphicsPipeline&) = delete;
        GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;

        VkPipeline getPipeline() const { return mPipeline; }
        VkPipelineLayout getPipelineLayout() const { return mPipelineLayout; }
        VkRenderPass getRenderPass() const { return mRenderPass; }

    private:
        void createRenderPass();
        void createGraphicsPipeline();

        Device::Device& mDevice;
        Device::SwapChain& mSwapChain;
        Shader& mVertexShader;
        Shader& mFragmentShader;

        VkRenderPass mRenderPass{VK_NULL_HANDLE};
        VkPipelineLayout mPipelineLayout{VK_NULL_HANDLE};
        VkPipeline mPipeline{VK_NULL_HANDLE};
    };
} // namespace Engine::Core::Pipeline
