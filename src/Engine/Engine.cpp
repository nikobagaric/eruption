#include "Engine/Engine.hpp"
#include "Engine/Core/Pipeline/PushConstantData.hpp"
#include "Engine/Core/Scene/Camera.hpp"
#include "Engine/Core/Scene/SphereCollider.hpp"
#include "Engine/Core/Scene/Transform.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <chrono>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace Engine {

namespace {
constexpr int32_t kMetalTextureIndex = 0;
constexpr int32_t kBlueCheckerTextureIndex = 1;
constexpr int32_t kRedCheckerTextureIndex = 2;
constexpr int32_t kGridTextureIndex = 3;

constexpr float kSphereRadius = 1.0f;
constexpr float kSphereRestY = 1.0f;         // sphere bottom touches the ground
constexpr float kSphereSpacing = 3.0f;       // x-offset of the side spheres
constexpr float kGroundHalfExtent = 10.0f;   // half of a 20x20 platform
constexpr float kGroundHalfThickness = 0.1f; // half of a 0.2-thick slab
constexpr float kGroundUvScale = 10.0f;      // tiles grid_placeholder.jpg
} // namespace
Engine::Engine(uint16_t width, uint16_t height, const std::string &title,
               const std::string &modelPath, bool enableSkyboxScene)
    : mModelPath(modelPath), mSkyboxSceneEnabled(enableSkyboxScene),
      mWindow(width, height, title) {
  init();
}

Engine::~Engine() {
  if (mDevice) {
    vkDeviceWaitIdle(mDevice->getDevice());
  }
}

void Engine::run() {
  mLastFrameTime = std::chrono::steady_clock::now();

  while (!glfwWindowShouldClose(mWindow.getGLFWWindow())) {
    glfwPollEvents();

    const auto now = std::chrono::steady_clock::now();
    const float dt = std::chrono::duration<float>(now - mLastFrameTime).count();
    mLastFrameTime = now;

    auto &cameraTransform =
        mRegistry.get<Core::Scene::Transform>(mCameraEntity);
    mCameraController.moveInPlaneXZ(mWindow.getGLFWWindow(), dt,
                                    cameraTransform);

    if (mSkyboxSceneEnabled) {
      const bool raycastKeyPressed =
          glfwGetKey(mWindow.getGLFWWindow(), GLFW_KEY_SPACE) == GLFW_PRESS;
      if (raycastKeyPressed && !mRaycastKeyWasPressed) {
        performRaycast();
      }
      mRaycastKeyWasPressed = raycastKeyPressed;
    }

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

  mSampleCount = mPhysicalDevice->getMaxUsableSampleCount();
  mDepthFormat = mPhysicalDevice->findDepthFormat();

  mVertexShader = std::make_unique<Core::Pipeline::Shader>(
      "shaders/triangle.vert.spv", *mDevice);
  mFragmentShader = std::make_unique<Core::Pipeline::Shader>(
      "shaders/triangle.frag.spv", *mDevice);

  createDescriptorSetLayout();

  mGraphicsPipeline =
      Core::Pipeline::GraphicsPipeline::Builder(*mDevice, *mSwapChain)
          .setShaders(*mVertexShader, *mFragmentShader)
          .setVertexInput<Core::Buffer::Vertex>()
          .setSampleCount(mSampleCount)
          .setDepthFormat(mDepthFormat)
          .addDescriptorSetLayout(mGlobalSetLayout->getDescriptorSetLayout())
          .addPushConstantRange<Core::Pipeline::PushConstantData>(
              VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
          .build();

  if (mSkyboxSceneEnabled) {
    mLitVertexShader = std::make_unique<Core::Pipeline::Shader>(
        "shaders/lit.vert.spv", *mDevice);
    mLitFragmentShader = std::make_unique<Core::Pipeline::Shader>(
        "shaders/lit.frag.spv", *mDevice);

    mLitPipeline =
        Core::Pipeline::GraphicsPipeline::Builder(*mDevice, *mSwapChain)
            .setShaders(*mLitVertexShader, *mLitFragmentShader)
            .setVertexInput<Core::Buffer::Vertex>()
            .setSampleCount(mSampleCount)
            .setDepthFormat(mDepthFormat)
            .addDescriptorSetLayout(mGlobalSetLayout->getDescriptorSetLayout())
            .addPushConstantRange<Core::Pipeline::PushConstantData>(
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
            .build();

    mSkyboxVertexShader = std::make_unique<Core::Pipeline::Shader>(
        "shaders/skybox.vert.spv", *mDevice);
    mSkyboxFragmentShader = std::make_unique<Core::Pipeline::Shader>(
        "shaders/skybox.frag.spv", *mDevice);

    mSkyboxPipeline =
        Core::Pipeline::GraphicsPipeline::Builder(*mDevice, *mSwapChain)
            .setShaders(*mSkyboxVertexShader, *mSkyboxFragmentShader)
            .setVertexInput<Core::Buffer::Vertex>()
            .setSampleCount(mSampleCount)
            .setDepthFormat(mDepthFormat)
            .setCullMode(VK_CULL_MODE_NONE)
            .setDepthWriteEnabled(false)
            .setDepthCompareOp(VK_COMPARE_OP_LESS_OR_EQUAL)
            .addDescriptorSetLayout(mGlobalSetLayout->getDescriptorSetLayout())
            .addPushConstantRange<Core::Pipeline::PushConstantData>(
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
            .build();
  }

  createDepthResources();
  createColorResources();
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
  createModel();
  createScene();
  createUniformBuffers();
  createDescriptorPool();
  createDescriptorSets();
  recordCommandBuffers();
}

void Engine::createDescriptorSetLayout() {
  mGlobalSetLayout =
      Core::Descriptor::DescriptorSetLayout::Builder(*mDevice)
          .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
          .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                      VK_SHADER_STAGE_FRAGMENT_BIT, Core::Image::kMaxTextures)
          .addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                      VK_SHADER_STAGE_FRAGMENT_BIT)
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
          .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, imageCount)
          .build();
}

void Engine::createDescriptorSets() {
  auto imageCount = static_cast<uint32_t>(mSwapChain->getImageViews().size());

  std::vector<VkDescriptorImageInfo> imageInfos(Core::Image::kMaxTextures);
  for (uint32_t slot = 0; slot < Core::Image::kMaxTextures; ++slot) {
    uint32_t textureIndex = slot < mTextures.size() ? slot : 0;
    imageInfos[slot] = mTextures[textureIndex]->getDescriptorImageInfo();
  }

  VkDescriptorImageInfo skyboxInfo = mSkyboxTexture->getDescriptorImageInfo();

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
             .writeImage(2, &skyboxInfo)
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

  VkExtent2D extent = mSwapChain->getExtent();
  const float aspect =
      static_cast<float>(extent.width) / static_cast<float>(extent.height);

  auto &cameraTransform = mRegistry.get<Core::Scene::Transform>(mCameraEntity);
  auto &camera = mRegistry.get<Core::Scene::Camera>(mCameraEntity);
  camera.setViewYXZ(cameraTransform.translation, cameraTransform.rotation);
  camera.setPerspectiveProjection(glm::radians(50.0f), aspect, 0.1f, 100.0f);

  Core::Buffer::UniformBufferObject ubo{};
  ubo.view = camera.getView();
  ubo.proj = camera.getProjection();
  ubo.lightPos = glm::vec4(2.0f, 2.0f, 2.0f, 1.0f);
  ubo.cameraPos = glm::vec4(cameraTransform.translation, 1.0f);
  ubo.time = time;

  mUniformBuffers[imageIndex]->write(&ubo, sizeof(ubo));
}

void Engine::createSyncObjects() {
  auto imageCount = static_cast<uint32_t>(mSwapChain->getImageViews().size());

  mImageAvailableSemaphores =
      std::make_unique<Core::Sync::SemaphorePool>(*mDevice, imageCount);
  mRenderFinishedSemaphores =
      std::make_unique<Core::Sync::SemaphorePool>(*mDevice, imageCount);
  mFencePool = std::make_unique<Core::Sync::FencePool>(*mDevice, imageCount);
}

void Engine::createDepthResources() {
  VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
  if (mDepthFormat == VK_FORMAT_D32_SFLOAT_S8_UINT ||
      mDepthFormat == VK_FORMAT_D24_UNORM_S8_UINT) {
    aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
  }

  mDepthImage = std::make_unique<Core::Image::Image>(
      *mDevice, mSwapChain->getExtent().width, mSwapChain->getExtent().height,
      mDepthFormat, VK_IMAGE_TILING_OPTIMAL,
      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 1, mSampleCount);

  mDepthImageView = std::make_unique<Core::Image::ImageView>(
      *mDevice, mDepthImage->getImage(), mDepthFormat, aspectMask);
}

void Engine::createColorResources() {
  if (mSampleCount == VK_SAMPLE_COUNT_1_BIT) {
    mColorImage.reset();
    mColorImageView.reset();
    return;
  }

  mColorImage = std::make_unique<Core::Image::Image>(
      *mDevice, mSwapChain->getExtent().width, mSwapChain->getExtent().height,
      mSwapChain->getImageFormat(), VK_IMAGE_TILING_OPTIMAL,
      VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
          VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 1, mSampleCount);

  mColorImageView = std::make_unique<Core::Image::ImageView>(
      *mDevice, mColorImage->getImage(), mSwapChain->getImageFormat(),
      VK_IMAGE_ASPECT_COLOR_BIT);
}

void Engine::createFramebuffers() {
  if (!mGraphicsPipeline)
    throw std::runtime_error(
        "Graphics pipeline must be initialized before creating framebuffers.");

  mFramebuffer = std::make_unique<Core::Pipeline::Framebuffer>(
      *mDevice, *mSwapChain, mGraphicsPipeline->getRenderPass(),
      mDepthImageView->getImageView(),
      mColorImageView ? mColorImageView->getImageView() : VK_NULL_HANDLE);
}

void Engine::recreateSwapChain() {
  int width = 0, height = 0;
  while (width == 0 || height == 0) {
    glfwGetFramebufferSize(mWindow.getGLFWWindow(), &width, &height);
    glfwWaitEvents();
  }

  vkDeviceWaitIdle(mDevice->getDevice());

  mFramebuffer.reset();
  mColorImageView.reset();
  mColorImage.reset();
  mDepthImageView.reset();
  mDepthImage.reset();
  mGraphicsPipeline.reset();
  mLitPipeline.reset();
  mSkyboxPipeline.reset();
  mCommandBuffer.reset();
  mImageAvailableSemaphores.reset();
  mRenderFinishedSemaphores.reset();
  mFencePool.reset();
  mSwapChain.reset();

  mSwapChain = std::make_unique<Core::Device::SwapChain>(
      *mDevice, *mPhysicalDevice, *mInstance, mWindow);
  mGraphicsPipeline =
      Core::Pipeline::GraphicsPipeline::Builder(*mDevice, *mSwapChain)
          .setShaders(*mVertexShader, *mFragmentShader)
          .setVertexInput<Core::Buffer::Vertex>()
          .setSampleCount(mSampleCount)
          .setDepthFormat(mDepthFormat)
          .addDescriptorSetLayout(mGlobalSetLayout->getDescriptorSetLayout())
          .addPushConstantRange<Core::Pipeline::PushConstantData>(
              VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
          .build();

  if (mSkyboxSceneEnabled) {
    mLitPipeline =
        Core::Pipeline::GraphicsPipeline::Builder(*mDevice, *mSwapChain)
            .setShaders(*mLitVertexShader, *mLitFragmentShader)
            .setVertexInput<Core::Buffer::Vertex>()
            .setSampleCount(mSampleCount)
            .setDepthFormat(mDepthFormat)
            .addDescriptorSetLayout(mGlobalSetLayout->getDescriptorSetLayout())
            .addPushConstantRange<Core::Pipeline::PushConstantData>(
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
            .build();

    mSkyboxPipeline =
        Core::Pipeline::GraphicsPipeline::Builder(*mDevice, *mSwapChain)
            .setShaders(*mSkyboxVertexShader, *mSkyboxFragmentShader)
            .setVertexInput<Core::Buffer::Vertex>()
            .setSampleCount(mSampleCount)
            .setDepthFormat(mDepthFormat)
            .setCullMode(VK_CULL_MODE_NONE)
            .setDepthWriteEnabled(false)
            .setDepthCompareOp(VK_COMPARE_OP_LESS_OR_EQUAL)
            .addDescriptorSetLayout(mGlobalSetLayout->getDescriptorSetLayout())
            .addPushConstantRange<Core::Pipeline::PushConstantData>(
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
            .build();
  }

  createDepthResources();
  createColorResources();
  createFramebuffers();

  mCommandBuffer = std::make_unique<Core::Commands::CommandBuffer>(
      *mDevice, *mCommandPool,
      static_cast<uint32_t>(mSwapChain->getImageViews().size()));

  createSyncObjects();
  recordCommandBuffers();

  mWindow.resetFramebufferResized();
}

void Engine::createModel() {
  mModel = std::make_unique<Core::Model::Model>(*mDevice, *mUploadContext,
                                                mModelPath);

  if (mSkyboxSceneEnabled) {
    mCubeModel = std::make_unique<Core::Model::Model>(*mDevice, *mUploadContext,
                                                      "models/cube.obj");
  }
}

void Engine::createScene() {
  mMeshEntity = mRegistry.create();
  mCameraEntity = mRegistry.create();
  if (mSkyboxSceneEnabled) {
    mBlueSphereEntity = mRegistry.create();
    mRedSphereEntity = mRegistry.create();
    mGroundEntity = mRegistry.create();
  }

  Core::Scene::Transform meshTransform{};
  Core::Scene::Transform cameraTransform{};
  if (mSkyboxSceneEnabled) {
    meshTransform.translation = glm::vec3(0.0f, kSphereRestY, 0.0f);
    cameraTransform.translation = glm::vec3(0.0f, 2.0f, 8.0f);
    cameraTransform.rotation = glm::vec3(-0.15f, 0.0f, 0.0f);
  } else {
    cameraTransform.translation = glm::vec3(0.0f, 0.0f, 2.5f);
  }
  mRegistry.emplace<Core::Scene::Transform>(mMeshEntity, meshTransform);
  mRegistry.emplace<Core::Scene::Transform>(mCameraEntity, cameraTransform);
  mRegistry.emplace<Core::Scene::Camera>(mCameraEntity);

  if (!mSkyboxSceneEnabled) {
    return;
  }

  mRegistry.emplace<Core::Scene::SphereCollider>(
      mMeshEntity, Core::Scene::SphereCollider{kSphereRadius, "metal sphere"});

  Core::Scene::Transform blueTransform{};
  blueTransform.translation = glm::vec3(-kSphereSpacing, kSphereRestY, 0.0f);
  mRegistry.emplace<Core::Scene::Transform>(mBlueSphereEntity, blueTransform);
  mRegistry.emplace<Core::Scene::SphereCollider>(
      mBlueSphereEntity,
      Core::Scene::SphereCollider{kSphereRadius, "blue checker sphere"});

  Core::Scene::Transform redTransform{};
  redTransform.translation = glm::vec3(kSphereSpacing, kSphereRestY, 0.0f);
  mRegistry.emplace<Core::Scene::Transform>(mRedSphereEntity, redTransform);
  mRegistry.emplace<Core::Scene::SphereCollider>(
      mRedSphereEntity,
      Core::Scene::SphereCollider{kSphereRadius, "red checker sphere"});

  Core::Scene::Transform groundTransform{};
  groundTransform.translation = glm::vec3(0.0f, -kGroundHalfThickness, 0.0f);
  groundTransform.scale =
      glm::vec3(kGroundHalfExtent * 2.0f, kGroundHalfThickness * 2.0f,
                kGroundHalfExtent * 2.0f);
  mRegistry.emplace<Core::Scene::Transform>(mGroundEntity, groundTransform);
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

    std::vector<VkClearValue> clearValues(
        mSampleCount == VK_SAMPLE_COUNT_1_BIT ? 2 : 3);
    clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo,
                         VK_SUBPASS_CONTENTS_INLINE);

    auto drawEntity = [&](Core::Pipeline::GraphicsPipeline &pipeline,
                          Core::Model::Model &model,
                          const glm::mat4 &modelMatrix, int32_t textureIndex,
                          float uvScale = 1.0f) {
      vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                        pipeline.getPipeline());
      vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                              pipeline.getPipelineLayout(), 0, 1,
                              &mDescriptorSets[i], 0, nullptr);

      Core::Pipeline::PushConstantData pushConstant{};
      pushConstant.model = modelMatrix;
      pushConstant.textureIndex = textureIndex;
      pushConstant.uvScale = uvScale;
      vkCmdPushConstants(commandBuffer, pipeline.getPipelineLayout(),
                         VK_SHADER_STAGE_VERTEX_BIT |
                             VK_SHADER_STAGE_FRAGMENT_BIT,
                         0, sizeof(pushConstant), &pushConstant);

      VkBuffer vertexBuffers[] = {model.getVertexBuffer()};
      VkDeviceSize offsets[] = {0};
      vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
      vkCmdBindIndexBuffer(commandBuffer, model.getIndexBuffer(), 0,
                           Core::Model::Model::getIndexType());

      vkCmdDrawIndexed(commandBuffer, model.getIndexCount(), 1, 0, 0, 0);
    };

    if (mSkyboxSceneEnabled) {
      drawEntity(*mSkyboxPipeline, *mCubeModel, glm::mat4(1.0f), 0);

      drawEntity(*mLitPipeline, *mModel,
                 mRegistry.get<Core::Scene::Transform>(mMeshEntity).matrix(),
                 kMetalTextureIndex);
      drawEntity(
          *mLitPipeline, *mModel,
          mRegistry.get<Core::Scene::Transform>(mBlueSphereEntity).matrix(),
          kBlueCheckerTextureIndex);
      drawEntity(
          *mLitPipeline, *mModel,
          mRegistry.get<Core::Scene::Transform>(mRedSphereEntity).matrix(),
          kRedCheckerTextureIndex);
      drawEntity(*mLitPipeline, *mCubeModel,
                 mRegistry.get<Core::Scene::Transform>(mGroundEntity).matrix(),
                 kGridTextureIndex, kGroundUvScale);
    } else {
      drawEntity(*mGraphicsPipeline, *mModel,
                 mRegistry.get<Core::Scene::Transform>(mMeshEntity).matrix(),
                 0);
    }

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
      mImageAvailableSemaphores->getSemaphore(mCurrentFrame), VK_NULL_HANDLE,
      &imageIndex);

  if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR ||
      acquireResult == VK_SUBOPTIMAL_KHR || mWindow.isFramebufferResized()) {
    recreateSwapChain();
    return;
  } else if (acquireResult != VK_SUCCESS) {
    throw std::runtime_error("failed to acquire swap chain image!");
  }

  updateUniformBuffer(imageIndex);

  VkSemaphore waitSemaphores[] = {
      mImageAvailableSemaphores->getSemaphore(mCurrentFrame)};
  VkSemaphore signalSemaphores[] = {
      mRenderFinishedSemaphores->getSemaphore(imageIndex)};

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
  const std::vector<std::string> texturePaths =
      mSkyboxSceneEnabled
          ? std::vector<std::string>{"textures/metal.jpg",
                                     "textures/checker_blue.ppm",
                                     "textures/checker_red.ppm",
                                     "textures/grid_placeholder.jpg"}
          : std::vector<std::string>{"textures/checker_red.ppm",
                                     "textures/checker_blue.ppm"};

  mTextures.clear();
  mTextures.reserve(texturePaths.size());
  for (const auto &path : texturePaths) {
    mTextures.push_back(std::make_unique<Core::Image::Texture>(
        *mDevice, *mPhysicalDevice, *mUploadContext, path));
  }

  mSkyboxTexture = std::make_unique<Core::Image::CubemapTexture>(
      *mDevice, *mPhysicalDevice, *mUploadContext,
      std::array<std::string, 6>{
          "textures/sky_110_2k/sky_110_cubemap_2k/px.png",
          "textures/sky_110_2k/sky_110_cubemap_2k/nx.png",
          "textures/sky_110_2k/sky_110_cubemap_2k/py.png",
          "textures/sky_110_2k/sky_110_cubemap_2k/ny.png",
          "textures/sky_110_2k/sky_110_cubemap_2k/pz.png",
          "textures/sky_110_2k/sky_110_cubemap_2k/nz.png",
      });
}

void Engine::performRaycast() {
  const auto &cameraTransform =
      mRegistry.get<Core::Scene::Transform>(mCameraEntity);
  const glm::vec3 origin = cameraTransform.translation;
  const glm::vec3 direction =
      Core::Scene::forwardFromRotation(cameraTransform.rotation);

  float closestDistance = std::numeric_limits<float>::max();
  std::string hitName;
  bool hit = false;

  mRegistry.each<Core::Scene::SphereCollider, Core::Scene::Transform>(
      [&](Core::ECS::Entity, Core::Scene::SphereCollider &collider,
          Core::Scene::Transform &transform) {
        const float radius = collider.radius * transform.scale.x;
        const glm::vec3 originToCenter = origin - transform.translation;
        const float b = glm::dot(originToCenter, direction);
        const float c =
            glm::dot(originToCenter, originToCenter) - radius * radius;
        const float discriminant = b * b - c;
        if (discriminant < 0.0f) {
          return;
        }

        const float distance = -b - std::sqrt(discriminant);
        if (distance >= 0.0f && distance < closestDistance) {
          closestDistance = distance;
          hitName = collider.name;
          hit = true;
        }
      });

  if (std::abs(direction.y) > std::numeric_limits<float>::epsilon()) {
    const float t = -origin.y / direction.y;
    if (t >= 0.0f && t < closestDistance) {
      const glm::vec3 point = origin + direction * t;
      if (std::abs(point.x) <= kGroundHalfExtent &&
          std::abs(point.z) <= kGroundHalfExtent) {
        closestDistance = t;
        hitName = "ground platform";
        hit = true;
      }
    }
  }

  if (hit) {
    std::cout << "Raycast hit: " << hitName << " at distance "
              << closestDistance << "\n";
  } else {
    std::cout << "Raycast: no hit\n";
  }
}

} // namespace Engine
