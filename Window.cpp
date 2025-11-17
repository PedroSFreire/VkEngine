#include "Window.h"


static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    app->setFramebufferResized(true);
}



Window::Window()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

Window::~Window()
{

    glfwDestroyWindow(window);
    glfwTerminate();
}





bool Window::shouldClose() const {
    return glfwWindowShouldClose(window);
}

 GLFWwindow* Window::getWindow() const {
    return window;
}

