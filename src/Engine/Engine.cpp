#include "Engine/Engine.hpp"

#include <array>
#include <stdexcept>

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

  mGraphicsPipeline = std::make_unique<Core::Pipeline::GraphicsPipeline>(
      *mDevice, *mSwapChain, *mVertexShader, *mFragmentShader);

  createFramebuffers();

  mCommandPool =
      std::make_unique<Core::Commands::CommandPool>(*mDevice, *mPhysicalDevice);
  mCommandBuffer = std::make_unique<Core::Commands::CommandBuffer>(
      *mDevice, *mCommandPool,
      static_cast<uint32_t>(mSwapChain->getImageViews().size()));

  mUploadContext = std::make_unique<Core::UploadContext>(
      *mDevice, *mCommandPool, mDevice->getGraphicsQueue());
  createSyncObjects();
  createVertexBuffer();
  recordCommandBuffers();
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
  mGraphicsPipeline = std::make_unique<Core::Pipeline::GraphicsPipeline>(
      *mDevice, *mSwapChain, *mVertexShader, *mFragmentShader);
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
  // Example vertex data for a triangle
  std::vector<Core::Buffer::Vertex> vertices = {
      {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}}, // Bottom vertex (red)
      {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},  // Top-right vertex (green)
      {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}  // Top-left vertex (blue)
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

} // namespace Engine
