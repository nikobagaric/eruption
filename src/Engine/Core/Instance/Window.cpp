#include "Engine/Core/Instance/Window.hpp"

#include <algorithm>

namespace Engine::Core::Instance
{
    /////////
    // EXT //
    /////////
    void Window::framebufferSizeCallback(GLFWwindow *window, int width, int height)
    {
        auto app = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
        app->mWidth = static_cast<uint16_t>(std::max(width, 0));
        app->mHeight = static_cast<uint16_t>(std::max(height, 0));
        app->mFramebufferResized = true;
    }

    /////////////////////
    // PRIVATE METHODS //
    /////////////////////

    void Window::init()
    {
        if (!glfwInit())
        {
            throw std::runtime_error("failed to initialize GLFW");
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        mWindow = glfwCreateWindow(mWidth, mHeight, mWindowName.c_str(), nullptr, nullptr);
        if (!mWindow)
        {
            glfwTerminate();
            throw std::runtime_error("failed to create GLFW window");
        }

        glfwSetWindowUserPointer(mWindow, this);
        glfwSetFramebufferSizeCallback(mWindow, framebufferSizeCallback);
    }

    void Window::loop()
    {
        while (!glfwWindowShouldClose(mWindow))
        {
            glfwPollEvents();
        }
    }

    void Window::createSurface(VkInstance instance, VkSurfaceKHR *surface)
    {
        if (glfwCreateWindowSurface(instance, mWindow, nullptr, surface) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create window surface!");
        }
    }

    ////////////////////
    // PUBLIC METHODS //
    ////////////////////

    Window::Window(uint16_t width, uint16_t height, const std::string &windowName) : mWidth{width}, mHeight{height}, mWindowName{windowName}
    {
        init();
    }

    Window::~Window()
    {
        glfwDestroyWindow(mWindow);
        glfwTerminate();
    }
}