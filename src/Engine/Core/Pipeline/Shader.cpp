#include "Engine/Core/Pipeline/Shader.hpp"

#ifdef __linux__
// special impl goes here, right now the only one
#include <fstream>
#else
#include <fstream>
#endif

#include <stdexcept>
#include <string>
#include <cstdint>

namespace Engine::Core::Pipeline {
    /////////////////////
    // PRIVATE METHODS //
    /////////////////////

    void Shader::readShader() {
#ifdef __linux__
#endif
        std::ifstream file(mPath, std::ios::ate | std::ios::binary);
        
        if(!file.is_open()) {
            throw std::runtime_error("failed to open shader file " + mPath);
        }
        mFileSize = (ssize_t) file.tellg();
        mBuffer.resize(mFileSize);

        file.seekg(0);
        file.read(mBuffer.data(), mFileSize);

        file.close();
    }

    void Shader::createShaderModule() {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = mBuffer.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(mBuffer.data());

        if(vkCreateShaderModule(mDevice.getDevice(), &createInfo, nullptr, &mModule) != VK_SUCCESS) {
            throw std::runtime_error("failure when creating shader from file: " + mPath);
        }
    }

    ////////////////////
    // PUBLIC METHODS //
    ////////////////////

    Shader::Shader(const std::string& path, Device::Device& device) : mPath{path}, mDevice{device} {
        readShader();
        createShaderModule();
    }

    Shader::~Shader() {
        vkDestroyShaderModule(mDevice.getDevice(), mModule, nullptr);
    }
}