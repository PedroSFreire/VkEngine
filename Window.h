#pragma once

#include <GLFW/glfw3.h>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

class Window
{
private:
	GLFWwindow* window;
	bool framebufferResized = false;

public:
	Window();
	~Window();
	Window(const Window&) = delete;

	GLFWwindow* const getWindow();
	
	bool getFramebufferResized() const { return framebufferResized; }
	void setFramebufferResized(bool val) { framebufferResized = val; }
	
	
	bool shouldClose();
};

