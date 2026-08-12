#include "Engine/Core/Descriptor/Descriptors.hpp"

#include <cassert>
#include <stdexcept>

// shoutout 2 lveEngine :P
namespace Engine::Core::Descriptor {

// ─────────────────────────────────────────────────────────────────────────────
// DescriptorSetLayout::Builder
// ─────────────────────────────────────────────────────────────────────────────

DescriptorSetLayout::Builder &DescriptorSetLayout::Builder::addBinding(
    uint32_t binding, VkDescriptorType descriptorType,
    VkShaderStageFlags stageFlags, uint32_t count) {
  assert(bindings.count(binding) == 0 && "Binding already in use");

  bindings[binding] = VkDescriptorSetLayoutBinding{
      .binding = binding,
      .descriptorType = descriptorType,
      .descriptorCount = count,
      .stageFlags = stageFlags,
      .pImmutableSamplers = nullptr,
  };

  return *this;
}

std::unique_ptr<DescriptorSetLayout>
DescriptorSetLayout::Builder::build() const {
  return std::make_unique<DescriptorSetLayout>(mDevice, bindings);
}

// ─────────────────────────────────────────────────────────────────────────────
// DescriptorSetLayout
// ─────────────────────────────────────────────────────────────────────────────

DescriptorSetLayout::DescriptorSetLayout(
    Device::Device &device,
    std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings)
    : mDevice{device}, mBindings{std::move(bindings)} {

  std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
  layoutBindings.reserve(mBindings.size());
  for (const auto &[key, val] : mBindings)
    layoutBindings.push_back(val);

  VkDescriptorSetLayoutCreateInfo info{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .bindingCount = static_cast<uint32_t>(layoutBindings.size()),
      .pBindings = layoutBindings.data(),
  };

  if (vkCreateDescriptorSetLayout(mDevice.getDevice(), &info, nullptr,
                                  &mDescriptorSetLayout) != VK_SUCCESS)
    throw std::runtime_error("Failed to create descriptor set layout!");
}

DescriptorSetLayout::~DescriptorSetLayout() {
  vkDestroyDescriptorSetLayout(mDevice.getDevice(), mDescriptorSetLayout,
                               nullptr);
}

// ─────────────────────────────────────────────────────────────────────────────
// DescriptorPool::Builder
// ─────────────────────────────────────────────────────────────────────────────

DescriptorPool::Builder &
DescriptorPool::Builder::addPoolSize(VkDescriptorType descriptorType,
                                     uint32_t count) {
  poolSizes.push_back({descriptorType, count});
  return *this;
}

DescriptorPool::Builder &
DescriptorPool::Builder::setPoolFlags(VkDescriptorPoolCreateFlags flags) {
  poolFlags = flags;
  return *this;
}

DescriptorPool::Builder &DescriptorPool::Builder::setMaxSets(uint32_t count) {
  maxSets = count;
  return *this;
}

std::unique_ptr<DescriptorPool> DescriptorPool::Builder::build() const {
  return std::make_unique<DescriptorPool>(mDevice, maxSets, poolFlags,
                                          poolSizes);
}

// ─────────────────────────────────────────────────────────────────────────────
// DescriptorPool
// ─────────────────────────────────────────────────────────────────────────────

DescriptorPool::DescriptorPool(
    Device::Device &device, uint32_t maxSets,
    VkDescriptorPoolCreateFlags poolFlags,
    const std::vector<VkDescriptorPoolSize> &poolSizes)
    : mDevice{device} {

  VkDescriptorPoolCreateInfo info{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .flags = poolFlags,
      .maxSets = maxSets,
      .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
      .pPoolSizes = poolSizes.data(),
  };

  if (vkCreateDescriptorPool(mDevice.getDevice(), &info, nullptr,
                             &mDescriptorPool) != VK_SUCCESS)
    throw std::runtime_error("Failed to create descriptor pool!");
}

DescriptorPool::~DescriptorPool() {
  vkDestroyDescriptorPool(mDevice.getDevice(), mDescriptorPool, nullptr);
}

bool DescriptorPool::allocateDescriptor(
    const VkDescriptorSetLayout descriptorSetLayout,
    VkDescriptorSet &descriptor) const {

  VkDescriptorSetAllocateInfo allocInfo{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .descriptorPool = mDescriptorPool,
      .descriptorSetCount = 1,
      .pSetLayouts = &descriptorSetLayout,
  };

  return vkAllocateDescriptorSets(mDevice.getDevice(), &allocInfo,
                                  &descriptor) == VK_SUCCESS;
}

void DescriptorPool::freeDescriptor(
    std::vector<VkDescriptorSet> &descriptors) const {
  vkFreeDescriptorSets(mDevice.getDevice(), mDescriptorPool,
                       static_cast<uint32_t>(descriptors.size()),
                       descriptors.data());
}

void DescriptorPool::resetPool() {
  vkResetDescriptorPool(mDevice.getDevice(), mDescriptorPool, 0);
}

// ─────────────────────────────────────────────────────────────────────────────
// DescriptorWriter
// ─────────────────────────────────────────────────────────────────────────────

DescriptorWriter::DescriptorWriter(DescriptorSetLayout &setLayout,
                                   DescriptorPool &pool)
    : mSetLayout{setLayout}, mPool{pool} {}

DescriptorWriter &
DescriptorWriter::writeBuffer(uint32_t binding,
                              VkDescriptorBufferInfo *bufferInfo) {
  assert(mSetLayout.mBindings.count(binding) == 1 &&
         "Layout does not contain specified binding");

  const auto &bindingDesc = mSetLayout.mBindings.at(binding);
  assert(bindingDesc.descriptorCount == 1 &&
         "Binding single descriptor info, but binding expects multiple");

  writes.push_back(VkWriteDescriptorSet{
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .dstBinding = binding,
      .descriptorCount = 1,
      .descriptorType = bindingDesc.descriptorType,
      .pBufferInfo = bufferInfo,
  });

  return *this;
}

DescriptorWriter &
DescriptorWriter::writeImage(uint32_t binding,
                             VkDescriptorImageInfo *imageInfo) {
  assert(mSetLayout.mBindings.count(binding) == 1 &&
         "Layout does not contain specified binding");

  const auto &bindingDesc = mSetLayout.mBindings.at(binding);
  assert(bindingDesc.descriptorCount == 1 &&
         "Binding single descriptor info, but binding expects multiple");

  writes.push_back(VkWriteDescriptorSet{
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .dstBinding = binding,
      .descriptorCount = 1,
      .descriptorType = bindingDesc.descriptorType,
      .pImageInfo = imageInfo,
  });

  return *this;
}

DescriptorWriter &
DescriptorWriter::writeImages(uint32_t binding,
                              VkDescriptorImageInfo *imageInfos,
                              uint32_t count) {
  assert(mSetLayout.mBindings.count(binding) == 1 &&
         "Layout does not contain specified binding");

  const auto &bindingDesc = mSetLayout.mBindings.at(binding);
  assert(count <= bindingDesc.descriptorCount &&
         "Writing more descriptors than the binding declares");

  writes.push_back(VkWriteDescriptorSet{
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .dstBinding = binding,
      .descriptorCount = count,
      .descriptorType = bindingDesc.descriptorType,
      .pImageInfo = imageInfos,
  });

  return *this;
}

bool DescriptorWriter::build(VkDescriptorSet &set) {
  if (!mPool.allocateDescriptor(mSetLayout.getDescriptorSetLayout(), set))
    return false;

  overwrite(set);
  return true;
}

void DescriptorWriter::overwrite(VkDescriptorSet &set) {
  for (auto &write : writes)
    write.dstSet = set;

  vkUpdateDescriptorSets(mPool.mDevice.getDevice(),
                         static_cast<uint32_t>(writes.size()), writes.data(), 0,
                         nullptr);
}

} // namespace Engine::Core::Descriptor
