#include "Engine/Core/Pipeline/Framebuffer.hpp"

namespace Engine::Core::Pipeline
{
    /////////////////////
    // PRIVATE METHODS //
    /////////////////////

    void Framebuffer::createFramebuffers()
    {
        mFramebuffers.resize(mSwapChain.getImageViews().size());

        for (size_t i = 0; i < mSwapChain.getImageViews().size(); i++)
        {
            VkImageView attachments[] = {mSwapChain.getImageViews()[i]};

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = mRenderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = mSwapChain.getExtent().width;
            framebufferInfo.height = mSwapChain.getExtent().height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(mDevice.getDevice(), &framebufferInfo, nullptr, &mFramebuffers[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create framebuffer!");
            }
        }
    }

    void Framebuffer::cleanupFramebuffers()
    {
        for (auto framebuffer : mFramebuffers)
        {
            vkDestroyFramebuffer(mDevice.getDevice(), framebuffer, nullptr);
        }
    }

    ////////////////////
    // PUBLIC METHODS //
    ////////////////////
    
    Framebuffer::Framebuffer(Device::Device& device, Device::SwapChain& swapChain, VkRenderPass renderPass)
        : mDevice(device), mSwapChain(swapChain), mRenderPass(renderPass)
    {
        createFramebuffers();
    }

    Framebuffer::~Framebuffer()
    {
        cleanupFramebuffers();
    }
} // namespace Engine::Core::Pipeline
