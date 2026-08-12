#pragma once

#include "Engine/Core/Buffer/IndexBuffer.hpp"
#include "Engine/Core/Buffer/UniformBuffer.hpp"
#include "Engine/Core/Buffer/VertexBuffer.hpp"
#include "Engine/Core/Commands/CommandBuffer.hpp"
#include "Engine/Core/Commands/CommandPool.hpp"
#include "Engine/Core/Descriptor/Descriptors.hpp"
#include "Engine/Core/Device/Device.hpp"
#include "Engine/Core/Device/PhysicalDevice.hpp"
#include "Engine/Core/Device/SwapChain.hpp"
#include "Engine/Core/Image/Texture.hpp"
#include "Engine/Core/Instance/Instance.hpp"
#include "Engine/Core/Instance/Window.hpp"
#include "Engine/Core/Pipeline/Framebuffer.hpp"
#include "Engine/Core/Pipeline/GraphicsPipeline.hpp"
#include "Engine/Core/Pipeline/Shader.hpp"
#include "Engine/Core/Sync/Fence.hpp"
#include "Engine/Core/Sync/Semaphore.hpp"
#include "Engine/Core/Utility/UploadContext.hpp"

#include <memory>
#include <vector>

namespace Engine {
class Engine {
public:
  explicit Engine(uint16_t width = 1280, uint16_t height = 720,
                  const std::string &title = "Eruption");
  ~Engine();

  Engine(const Engine &) = delete;
  Engine &operator=(const Engine &) = delete;

  void run();

private:
  void init();
  void recreateSwapChain();
  void createSyncObjects();
  void createTextures();
  void createFramebuffers();
  void createVertexBuffer();
  void createDescriptorSetLayout();
  void createUniformBuffers();
  void createDescriptorPool();
  void createDescriptorSets();
  void updateUniformBuffer(uint32_t imageIndex);
  void recordCommandBuffers();
  void drawFrame();

  Core::Instance::Window mWindow;
  std::unique_ptr<Core::Instance::Instance> mInstance;
  std::unique_ptr<Core::Device::PhysicalDevice> mPhysicalDevice;
  std::unique_ptr<Core::Device::Device> mDevice;
  std::unique_ptr<Core::UploadContext> mUploadContext;
  std::unique_ptr<Core::Device::SwapChain> mSwapChain;
  std::unique_ptr<Core::Pipeline::Shader> mVertexShader;
  std::unique_ptr<Core::Pipeline::Shader> mFragmentShader;
  std::unique_ptr<Core::Pipeline::GraphicsPipeline> mGraphicsPipeline;
  std::unique_ptr<Core::Pipeline::Framebuffer> mFramebuffer;
  std::unique_ptr<Core::Commands::CommandPool> mCommandPool;
  std::unique_ptr<Core::Buffer::VertexBuffer<Core::Buffer::Vertex>>
      mVertexBuffer;
  std::unique_ptr<Core::Buffer::IndexBuffer> mIndexBuffer;
  std::unique_ptr<Core::Commands::CommandBuffer> mCommandBuffer;
  std::unique_ptr<Core::Sync::SemaphorePool> mSemaphorePool;
  std::unique_ptr<Core::Sync::FencePool> mFencePool;

  std::unique_ptr<Core::Descriptor::DescriptorSetLayout> mGlobalSetLayout;
  std::unique_ptr<Core::Descriptor::DescriptorPool> mDescriptorPool;
  std::vector<std::unique_ptr<Core::Buffer::UniformBuffer>> mUniformBuffers;
  std::vector<VkDescriptorSet> mDescriptorSets;
  std::vector<std::unique_ptr<Core::Image::Texture>> mTextures;

  size_t mCurrentFrame = 0;
};
} // namespace Engine
