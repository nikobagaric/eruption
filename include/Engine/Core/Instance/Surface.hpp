#pragma once

#include <vulkan/vulkan.h>

namespace Engine::Core::Instance {
    class Surface {
    public:

        explicit Surface();
        ~Surface();

        VkSurfaceKHR getSurface() const { return mSurface; }
    private:
        VkSurfaceKHR mSurface;
    };
}