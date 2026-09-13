#include "Engine/Core/Pipeline/Framebuffer.hpp"

namespace Engine::Core::Pipeline
{
    /////////////////////
    // PRIVATE METHODS //
    /////////////////////

    void Framebuffer::createFramebuffers()
    {
        mFramebuffers.resize(mSwapChain.getImageViews().size());

        const bool msaaEnabled = mColorImageView != VK_NULL_HANDLE;

        for (size_t i = 0; i < mSwapChain.getImageViews().size(); i++)
        {
            std::vector<VkImageView> attachments;
            if (msaaEnabled)
            {
                attachments = {mColorImageView, mDepthImageView, mSwapChain.getImageViews()[i]};
            }
            else
            {
                attachments = {mSwapChain.getImageViews()[i], mDepthImageView};
            }

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = mRenderPass;
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
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
    
    Framebuffer::Framebuffer(Device::Device& device, Device::SwapChain& swapChain, VkRenderPass renderPass,
                             VkImageView depthImageView, VkImageView colorImageView)
        : mDevice(device), mSwapChain(swapChain), mRenderPass(renderPass),
          mDepthImageView(depthImageView), mColorImageView(colorImageView)
    {
        createFramebuffers();
    }

    Framebuffer::~Framebuffer()
    {
        cleanupFramebuffers();
    }
} // namespace Engine::Core::Pipeline
