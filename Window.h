#pragma once

#include <GLFW/glfw3.h>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

class Window
{
private:
	GLFWwindow* window;

public:
	Window();
	~Window();
	GLFWwindow* const getWindow();
	bool shouldClose();
};

