#pragma once

#include "Engine/Core/Device/Device.hpp"
#include "Engine/Core/Device/SwapChain.hpp"
#include "Shader.hpp"

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

namespace Engine::Core::Pipeline
{
    struct PipelineConfigInfo {
        PipelineConfigInfo() = default;
        PipelineConfigInfo(const PipelineConfigInfo&) = delete;
        PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

        std::vector<VkVertexInputBindingDescription> bindingDescriptions{};
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
        VkPipelineRasterizationStateCreateInfo rasterizationInfo{};
        VkPipelineMultisampleStateCreateInfo multisampleInfo{};
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        VkPipelineColorBlendStateCreateInfo colorBlendInfo{};
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};
        VkFormat depthFormat{VK_FORMAT_UNDEFINED};

        std::vector<VkDescriptorSetLayout> setLayouts{};
        std::vector<VkPushConstantRange> pushConstantRanges{};
    };

    class GraphicsPipeline {
    public:
        class Builder {
        public:
            Builder(Device::Device& device, Device::SwapChain& swapChain);

            Builder& setShaders(Shader& vertexShader, Shader& fragmentShader);

            template <typename VertexType>
            Builder& setVertexInput() {
                mConfigInfo.bindingDescriptions = {VertexType::getBindingDescription()};
                auto attributes = VertexType::getAttributeDescriptions();
                mConfigInfo.attributeDescriptions.assign(attributes.begin(), attributes.end());
                return *this;
            }

            Builder& addDescriptorSetLayout(VkDescriptorSetLayout setLayout);
            Builder& setSampleCount(VkSampleCountFlagBits sampleCount);
            Builder& setDepthFormat(VkFormat depthFormat);

            template <typename PushConstantType>
            Builder& addPushConstantRange(VkShaderStageFlags stageFlags, uint32_t offset = 0) {
                mConfigInfo.pushConstantRanges.push_back(VkPushConstantRange{
                    .stageFlags = stageFlags,
                    .offset = offset,
                    .size = sizeof(PushConstantType),
                });
                return *this;
            }

            std::unique_ptr<GraphicsPipeline> build();

        private:
            Device::Device& mDevice;
            Device::SwapChain& mSwapChain;
            Shader* mVertexShader{nullptr};
            Shader* mFragmentShader{nullptr};
            PipelineConfigInfo mConfigInfo{};
        };

        ~GraphicsPipeline();

        GraphicsPipeline(const GraphicsPipeline&) = delete;
        GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;

        VkPipeline getPipeline() const { return mPipeline; }
        VkPipelineLayout getPipelineLayout() const { return mPipelineLayout; }
        VkRenderPass getRenderPass() const { return mRenderPass; }

    private:
        GraphicsPipeline(Device::Device& device, Device::SwapChain& swapChain,
                         Shader& vertexShader, Shader& fragmentShader,
                         const PipelineConfigInfo& configInfo);

        static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);

        void createRenderPass(const PipelineConfigInfo& configInfo);
        void createGraphicsPipeline(Shader& vertexShader, Shader& fragmentShader,
                                    const PipelineConfigInfo& configInfo);

        Device::Device& mDevice;
        Device::SwapChain& mSwapChain;

        VkRenderPass mRenderPass{VK_NULL_HANDLE};
        VkPipelineLayout mPipelineLayout{VK_NULL_HANDLE};
        VkPipeline mPipeline{VK_NULL_HANDLE};
    };
} // namespace Engine::Core::Pipeline
