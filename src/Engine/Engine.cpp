#include "Engine/Core/Pipeline/PushConstantData.hpp"
#include "Engine/Engine.hpp"

#include <chrono>
#include <stdexcept>
#include <string>
#include <vector>

namespace Engine {
Engine::Engine(uint16_t width, uint16_t height, const std::string &title)
    : mWindow(width, height, title) {
  init();
}

Engine::~Engine() {
  if (mDevice) {
    vkDeviceWaitIdle(mDevice->getDevice());
  }
}

void Engine::run() {
  while (!glfwWindowShouldClose(mWindow.getGLFWWindow())) {
    glfwPollEvents();

    if (mWindow.isFramebufferResized()) {
      recreateSwapChain();
    }

    drawFrame();
  }

  vkDeviceWaitIdle(mDevice->getDevice());
}

void Engine::init() {
  mInstance = std::make_unique<Core::Instance::Instance>();
  mInstance->createSurface(mWindow);

  mPhysicalDevice =
      std::make_unique<Core::Device::PhysicalDevice>(*mInstance, mWindow);
  mDevice = std::make_unique<Core::Device::Device>(*mPhysicalDevice);
  mSwapChain = std::make_unique<Core::Device::SwapChain>(
      *mDevice, *mPhysicalDevice, *mInstance, mWindow);

  mVertexShader = std::make_unique<Core::Pipeline::Shader>(
      "shaders/triangle.vert.spv", *mDevice);
  mFragmentShader = std::make_unique<Core::Pipeline::Shader>(
      "shaders/triangle.frag.spv", *mDevice);

  createDescriptorSetLayout();

  mGraphicsPipeline = Core::Pipeline::GraphicsPipeline::Builder(*mDevice, *mSwapChain)
                          .setShaders(*mVertexShader, *mFragmentShader)
                          .setVertexInput<Core::Buffer::Vertex>()
                          .addDescriptorSetLayout(
                              mGlobalSetLayout->getDescriptorSetLayout())
                          .addPushConstantRange<Core::Pipeline::PushConstantData>(
                              VK_SHADER_STAGE_VERTEX_BIT |
                              VK_SHADER_STAGE_FRAGMENT_BIT)
                          .build();

  createFramebuffers();

  mCommandPool =
      std::make_unique<Core::Commands::CommandPool>(*mDevice, *mPhysicalDevice);
  mCommandBuffer = std::make_unique<Core::Commands::CommandBuffer>(
      *mDevice, *mCommandPool,
      static_cast<uint32_t>(mSwapChain->getImageViews().size()));

  mUploadContext = std::make_unique<Core::UploadContext>(
      *mDevice, *mCommandPool, mDevice->getGraphicsQueue());
  createSyncObjects();
  createTextures();
  createVertexBuffer();
  createUniformBuffers();
  createDescriptorPool();
  createDescriptorSets();
  recordCommandBuffers();
}

void Engine::createDescriptorSetLayout() {
  mGlobalSetLayout =
      Core::Descriptor::DescriptorSetLayout::Builder(*mDevice)
          .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                      VK_SHADER_STAGE_VERTEX_BIT)
          .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                      VK_SHADER_STAGE_FRAGMENT_BIT,
                      Core::Image::kMaxTextures)
          .build();
}

void Engine::createUniformBuffers() {
  auto imageCount = static_cast<uint32_t>(mSwapChain->getImageViews().size());

  mUniformBuffers.clear();
  mUniformBuffers.reserve(imageCount);
  for (uint32_t i = 0; i < imageCount; ++i) {
    mUniformBuffers.push_back(std::make_unique<Core::Buffer::UniformBuffer>(
        *mDevice, sizeof(Core::Buffer::UniformBufferObject)));
  }
}

void Engine::createDescriptorPool() {
  auto imageCount = static_cast<uint32_t>(mSwapChain->getImageViews().size());

  mDescriptorPool =
      Core::Descriptor::DescriptorPool::Builder(*mDevice)
          .setMaxSets(imageCount)
          .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, imageCount)
          .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                       Core::Image::kMaxTextures * imageCount)
          .build();
}

void Engine::createDescriptorSets() {
  auto imageCount = static_cast<uint32_t>(mSwapChain->getImageViews().size());

  std::vector<VkDescriptorImageInfo> imageInfos(Core::Image::kMaxTextures);
  for (uint32_t slot = 0; slot < Core::Image::kMaxTextures; ++slot) {
    uint32_t textureIndex = slot < mTextures.size() ? slot : 0;
    imageInfos[slot] = mTextures[textureIndex]->getDescriptorImageInfo();
  }

  mDescriptorSets.resize(imageCount);
  for (uint32_t i = 0; i < imageCount; ++i) {
    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = mUniformBuffers[i]->getBuffer();
    bufferInfo.offset = 0;
    bufferInfo.range = mUniformBuffers[i]->getSize();

    Core::Descriptor::DescriptorWriter writer(*mGlobalSetLayout,
                                              *mDescriptorPool);
    if (!writer.writeBuffer(0, &bufferInfo)
             .writeImages(1, imageInfos.data(),
                          static_cast<uint32_t>(imageInfos.size()))
             .build(mDescriptorSets[i])) {
      throw std::runtime_error("failed to allocate descriptor set!");
    }
  }
}

void Engine::updateUniformBuffer(uint32_t imageIndex) {
  static const auto startTime = std::chrono::high_resolution_clock::now();
  float time = std::chrono::duration<float>(
                   std::chrono::high_resolution_clock::now() - startTime)
                   .count();

  Core::Buffer::UniformBufferObject ubo{};
  ubo.view = glm::mat4(1.0f);
  ubo.proj = glm::mat4(1.0f);
  ubo.lightPos = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
  ubo.time = time;

  mUniformBuffers[imageIndex]->write(&ubo, sizeof(ubo));
}

void Engine::createSyncObjects() {
  mSemaphorePool = std::make_unique<Core::Sync::SemaphorePool>(*mDevice, 2);
  mFencePool = std::make_unique<Core::Sync::FencePool>(
      *mDevice, static_cast<uint32_t>(mSwapChain->getImageViews().size()));
}

void Engine::createFramebuffers() {
  if (!mGraphicsPipeline)
    throw std::runtime_error(
        "Graphics pipeline must be initialized before creating framebuffers.");

  mFramebuffer = std::make_unique<Core::Pipeline::Framebuffer>(
      *mDevice, *mSwapChain, mGraphicsPipeline->getRenderPass());
}

void Engine::recreateSwapChain() {
  int width = 0, height = 0;
  while (width == 0 || height == 0) {
    glfwGetFramebufferSize(mWindow.getGLFWWindow(), &width, &height);
    glfwWaitEvents();
  }

  vkDeviceWaitIdle(mDevice->getDevice());

  mFramebuffer.reset();
  mGraphicsPipeline.reset();
  mCommandBuffer.reset();
  mSemaphorePool.reset();
  mFencePool.reset();
  mSwapChain.reset();

  mSwapChain = std::make_unique<Core::Device::SwapChain>(
      *mDevice, *mPhysicalDevice, *mInstance, mWindow);
  mGraphicsPipeline = Core::Pipeline::GraphicsPipeline::Builder(*mDevice, *mSwapChain)
                          .setShaders(*mVertexShader, *mFragmentShader)
                          .setVertexInput<Core::Buffer::Vertex>()
                          .addDescriptorSetLayout(
                              mGlobalSetLayout->getDescriptorSetLayout())
                          .addPushConstantRange<Core::Pipeline::PushConstantData>(
                              VK_SHADER_STAGE_VERTEX_BIT |
                              VK_SHADER_STAGE_FRAGMENT_BIT)
                          .build();
  mFramebuffer = std::make_unique<Core::Pipeline::Framebuffer>(
      *mDevice, *mSwapChain, mGraphicsPipeline->getRenderPass());

  mCommandBuffer = std::make_unique<Core::Commands::CommandBuffer>(
      *mDevice, *mCommandPool,
      static_cast<uint32_t>(mSwapChain->getImageViews().size()));

  createSyncObjects();
  recordCommandBuffers();

  mWindow.resetFramebufferResized();
}

void Engine::createVertexBuffer() {
  std::vector<Core::Buffer::Vertex> vertices = {
      {{0.0f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.5f, 1.0f}},
      {{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
      {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
  };

  mVertexBuffer =
      std::make_unique<Core::Buffer::VertexBuffer<Core::Buffer::Vertex>>(
          *mDevice, *mUploadContext, vertices);
}

void Engine::recordCommandBuffers() {
  auto framebufferCount = mFramebuffer->getFramebuffers().size();
  auto commandBufferCount =
      static_cast<uint32_t>(mCommandBuffer->getCommandBuffers().size());

  if (framebufferCount != commandBufferCount) {
    throw std::runtime_error(
        "Framebuffers and command buffers must match in count.");
  }

  for (uint32_t i = 0; i < commandBufferCount; ++i) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

    VkCommandBuffer commandBuffer = mCommandBuffer->getCommandBuffer(i);
    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
      throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = mGraphicsPipeline->getRenderPass();
    renderPassInfo.framebuffer = mFramebuffer->getFramebuffers()[i];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = mSwapChain->getExtent();

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo,
                         VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      mGraphicsPipeline->getPipeline());

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            mGraphicsPipeline->getPipelineLayout(), 0, 1,
                            &mDescriptorSets[i], 0, nullptr);

    Core::Pipeline::PushConstantData pushConstant{};
    pushConstant.model = glm::mat4(1.0f);
    pushConstant.textureIndex = 0;
    vkCmdPushConstants(commandBuffer, mGraphicsPipeline->getPipelineLayout(),
                       VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                       0, sizeof(pushConstant), &pushConstant);

    VkBuffer vertexBuffers[] = {mVertexBuffer->getBuffer()};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    vkCmdDraw(commandBuffer, mVertexBuffer->getVertexCount(), 1, 0, 0);
    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
      throw std::runtime_error("failed to record command buffer!");
    }
  }
}

void Engine::drawFrame() {
  auto &fences = mFencePool->getFences();
  uint32_t imageIndex = 0;

  if (vkWaitForFences(mDevice->getDevice(), 1, &fences[mCurrentFrame], VK_TRUE,
                      UINT64_MAX) != VK_SUCCESS) {
    throw std::runtime_error("failed to wait for fence!");
  }

  if (vkResetFences(mDevice->getDevice(), 1, &fences[mCurrentFrame]) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to reset fence!");
  }

  VkResult acquireResult = vkAcquireNextImageKHR(
      mDevice->getDevice(), mSwapChain->getSwapChain(), UINT64_MAX,
      mSemaphorePool->getSemaphore(0), VK_NULL_HANDLE, &imageIndex);

  if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR ||
      acquireResult == VK_SUBOPTIMAL_KHR || mWindow.isFramebufferResized()) {
    recreateSwapChain();
    return;
  } else if (acquireResult != VK_SUCCESS) {
    throw std::runtime_error("failed to acquire swap chain image!");
  }

  updateUniformBuffer(imageIndex);

  VkSemaphore waitSemaphores[] = {mSemaphorePool->getSemaphore(0)};
  VkSemaphore signalSemaphores[] = {mSemaphorePool->getSemaphore(1)};

  VkPipelineStageFlags waitStages[] = {
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

  VkCommandBuffer commandBuffers[] = {
      mCommandBuffer->getCommandBuffer(imageIndex)};

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = waitSemaphores;
  submitInfo.pWaitDstStageMask = waitStages;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = commandBuffers;
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalSemaphores;

  if (vkQueueSubmit(mDevice->getGraphicsQueue(), 1, &submitInfo,
                    fences[mCurrentFrame]) != VK_SUCCESS) {
    throw std::runtime_error("failed to submit draw command buffer!");
  }

  VkPresentInfoKHR presentInfo{};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = signalSemaphores;

  VkSwapchainKHR swapChains[] = {mSwapChain->getSwapChain()};
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = swapChains;
  presentInfo.pImageIndices = &imageIndex;

  VkResult presentResult =
      vkQueuePresentKHR(mDevice->getPresentQueue(), &presentInfo);

  if (presentResult == VK_ERROR_OUT_OF_DATE_KHR ||
      presentResult == VK_SUBOPTIMAL_KHR || mWindow.isFramebufferResized()) {
    recreateSwapChain();
    return;
  } else if (presentResult != VK_SUCCESS) {
    throw std::runtime_error("failed to present swap chain image!");
  }

  mCurrentFrame = (mCurrentFrame + 1) % fences.size();
}

void Engine::createTextures() {
  static const std::vector<std::string> texturePaths = {
      "textures/checker_red.ppm",
      "textures/checker_blue.ppm",
  };

  mTextures.clear();
  mTextures.reserve(texturePaths.size());
  for (const auto &path : texturePaths) {
    mTextures.push_back(std::make_unique<Core::Image::Texture>(
        *mDevice, *mPhysicalDevice, *mUploadContext, path));
  }
}

} // namespace Engine
