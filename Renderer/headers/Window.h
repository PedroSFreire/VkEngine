#pragma once

#include <GLFW/glfw3.h>

const uint32_t WIDTH = 1920;
const uint32_t HEIGHT = 1080;

class Window
{

public:
	Window();
	~Window();
	Window(const Window&) = delete;

	GLFWwindow* getWindow() const;
	
	bool getFramebufferResized() const { return framebufferResized; }

	void setFramebufferResized(const bool val) { framebufferResized = val; }

	bool shouldClose() const;

private:
	GLFWwindow* window;

	bool framebufferResized = false;
};

