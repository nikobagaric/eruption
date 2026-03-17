#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstdint>
#include <string>

namespace Engine::Core::Instance {
    class Window {
    public:
        explicit Window(uint16_t width, uint16_t height, const std::string& windowName = "Window");
        ~Window();

        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

        Window(Window&&) noexcept = default;
        Window& operator=(Window&&) noexcept = default;

        GLFWwindow *getGLFWWindow() const { return mWindow; }
    private:

        void init();
        void loop();

        uint16_t mWidth;
        uint16_t mHeight;

        std::string mWindowName;

        GLFWwindow* mWindow;
    };
}
