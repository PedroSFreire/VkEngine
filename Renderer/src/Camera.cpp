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