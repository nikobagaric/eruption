#pragma once

#include "Engine/Core/Device/Device.hpp"

#include <vulkan/vulkan.hpp>
#include <string>

namespace Engine::Core::Pipeline
{
    class Shader {
    public:
        explicit Shader(const std::string& path, Device::Device& device);
        ~Shader();

        Shader(const Shader&) = delete;
        Shader& operator=(const Shader&) = delete;

        Shader(Shader&&) noexcept = default;
        Shader& operator=(Shader&&) noexcept = default;

        VkShaderModule getModule() const { return mModule; }

    private:

        Device::Device &mDevice;
        
        std::string mPath;

        ssize_t mFileSize;
        std::vector<char> mBuffer;

        VkShaderModule mModule;

        // MAKE A FILE IMPLEMENTATION FOR SPEED (WILL BE LINUX ONLY)
        // FOR A RAPID WINDOWS / MACOS IMPL FILE* WILL BE FAST ENOUGH
        // TO BE PLATFORM AGNOSTIC ADD PREPROCESSORS IN THE PRIVATE IMPL
        void readShader();

        void createShaderModule();
    };
} // namespace Engine::Core::Pipeline
