#include "Engine/Core/Buffer/Buffer.hpp"
#include <stdexcept>

namespace Engine::Core::Buffer
{
    /////////////////////
    // PRIVATE METHODS //
    /////////////////////

    void Buffer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
    {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(mDevice.getDevice(), &bufferInfo, nullptr, &mBuffer) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create buffer!");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(mDevice.getDevice(), mBuffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = mDevice.findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(mDevice.getDevice(), &allocInfo, nullptr, &mBufferMemory) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate buffer memory!");
        }

        vkBindBufferMemory(mDevice.getDevice(), mBuffer, mBufferMemory, 0);
    }

    ////////////////////
    // PUBLIC METHODS //
    ////////////////////

    Buffer::Buffer(Device::Device &device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
        : mDevice(device)
    {
        createBuffer(size, usage, properties);
    }

    Buffer::~Buffer()
    {
        if (mBuffer != VK_NULL_HANDLE)
        {
            vkDestroyBuffer(mDevice.getDevice(), mBuffer, nullptr);
            mBuffer = VK_NULL_HANDLE;
        }
        if (mBufferMemory != VK_NULL_HANDLE)
        {
            vkFreeMemory(mDevice.getDevice(), mBufferMemory, nullptr);
            mBufferMemory = VK_NULL_HANDLE;
        }
    }
} // namespace Engine::Core::Buffer
