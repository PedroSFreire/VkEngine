#include "..\headers\Camera.h"

void Camera::updateCam(double mouseX, double mouseY) {

	if (firstMouse)
	{
		lastMouseX = mouseX;
		lastMouseY = mouseY;
		firstMouse = false;
		return;
	}

	float offSetYaw = static_cast<float>((mouseX - lastMouseX) * camSensivity);
	float offSetPitch = static_cast<float>((mouseY - lastMouseY) * camSensivity);

	pitch -= offSetPitch;
	yaw += offSetYaw;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;



	camDirection.x = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
	camDirection.y = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
	camDirection.z = sin(glm::radians(pitch));

	camDirection = glm::normalize(camDirection);
	camRight = glm::normalize(glm::cross(camDirection, glm::vec3(0.0f, 0.0f, 1.0f)));
	camUp = glm::normalize(glm::cross(camRight, camDirection));

	lastMouseX = mouseX;
	lastMouseY = mouseY;
};


void Camera::processInput(float deltaTime, Window& window) {
	// Implement camera movement and input processing here
	if (glfwGetKey(window.getWindow(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window.getWindow(), true);
	double mouseX, mouseY;
	glfwGetCursorPos(window.getWindow(), &mouseX, &mouseY);

	updateCam(mouseX, mouseY);


	//WASD movement
	if (glfwGetKey(window.getWindow(), GLFW_KEY_W) == GLFW_PRESS)
		goForward(2.0f * deltaTime);
	if (glfwGetKey(window.getWindow(), GLFW_KEY_S) == GLFW_PRESS)
		goForward(-2.0f * deltaTime);
	if (glfwGetKey(window.getWindow(), GLFW_KEY_D) == GLFW_PRESS)
		goRight(2.0f * deltaTime);
	if (glfwGetKey(window.getWindow(), GLFW_KEY_A) == GLFW_PRESS)
		goRight(-2.0f * deltaTime);
	if (glfwGetKey(window.getWindow(), GLFW_KEY_SPACE) == GLFW_PRESS)
		goUp(1.0f * deltaTime);
	if (glfwGetKey(window.getWindow(), GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		goUp(-1.0f * deltaTime);


}