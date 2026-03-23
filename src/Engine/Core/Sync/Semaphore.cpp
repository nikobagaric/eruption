#include "Engine/Core/Sync/Semaphore.hpp"

#include <stdexcept>

namespace Engine::Core::Sync
{
    /////////////////////////
    // SEMAPHORE METHODS   //
    /////////////////////////

    /////////////////////
    // PRIVATE METHODS //
    /////////////////////

    void Semaphore::createSemaphore()
    {
        VkSemaphoreCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        if (vkCreateSemaphore(mDevice.getDevice(), &createInfo, nullptr, &mSemaphore) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create semaphore!");
        }
    }

    ////////////////////
    // PUBLIC METHODS //
    ////////////////////

    Semaphore::Semaphore(Device::Device& device)
        : mDevice(device)
    {
        createSemaphore();
    }

    Semaphore::~Semaphore()
    {
        if (mSemaphore != VK_NULL_HANDLE)
        {
            vkDestroySemaphore(mDevice.getDevice(), mSemaphore, nullptr);
            mSemaphore = VK_NULL_HANDLE;
        }
    }

    ///////////////////////////
    // SEMAPHORE POOL METHODS  //
    ///////////////////////////

    /////////////////////
    // PRIVATE METHODS //
    /////////////////////

    void SemaphorePool::createSemaphores()
    {
        mSemaphores.resize(mPoolSize);

        VkSemaphoreCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        for (uint32_t i = 0; i < mPoolSize; ++i)
        {
            if (vkCreateSemaphore(mDevice.getDevice(), &createInfo, nullptr, &mSemaphores[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create semaphore pool!");
            }
        }
    }

    ////////////////////
    // PUBLIC METHODS //
    ////////////////////

    SemaphorePool::SemaphorePool(Device::Device& device, uint32_t poolSize)
        : mDevice(device), mPoolSize(poolSize)
    {
        createSemaphores();
    }

    SemaphorePool::~SemaphorePool()
    {
        for (VkSemaphore semaphore : mSemaphores)
        {
            if (semaphore != VK_NULL_HANDLE)
            {
                vkDestroySemaphore(mDevice.getDevice(), semaphore, nullptr);
            }
        }
        mSemaphores.clear();
    }

} // namespace Engine::Core::Sync
