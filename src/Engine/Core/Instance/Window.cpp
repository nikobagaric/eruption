#include "Engine/Core/Instance/Window.hpp"

namespace Engine::Core::Instance
{
    /////////////////////
    // PRIVATE METHODS //
    /////////////////////

    void Window::init()
    {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // @todo: TO BE CHANGED TO GLFW_TRUE

        mWindow = glfwCreateWindow(mWidth, mHeight, mWindowName.c_str(), nullptr, nullptr);

        loop();
    }

    void Window::loop() {
        while(!glfwWindowShouldClose(mWindow)) {
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

    Window::~Window() {
        glfwDestroyWindow(mWindow);
        glfwTerminate();
    }
}