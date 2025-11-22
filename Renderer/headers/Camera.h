#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#
class Camera
{


private:
	glm::vec3 camPos = glm::vec3(2.0f, 2.0f, 2.0f);
	glm::vec3 camDirection = glm::normalize(glm::vec3(0.0f, 0.0f, 0.0f) - camPos);
	glm::vec3 camUp = glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec3 camRight = glm::normalize(glm::cross(camDirection, camUp));

	double lastMouseX = 0;
	double lastMouseY = 0;
	float sensitivity = 0.001f;
	float camSensivity = 0.1f;
	bool firstMouse = true;

	float pitch = -30;
	float yaw = -135;
	

public:
	Camera() = default;
	void updateCam(double mouseX, double mouseY) {

		if(firstMouse)
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

		std::cout << "Pitch: " << pitch << " Yaw: " << yaw << std::endl;
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


	glm::vec3& getPos() { return camPos; };
	glm::vec3& getDirection() { return camDirection; }
	glm::vec3& getUp() { return camUp; }
	void goForward(float d) { camPos += d * camDirection; }
	void goUp(float d) {
		camPos += d * camUp; 
	};
	void goRight(float d) {
		camRight = glm::normalize(glm::cross(camDirection, camUp));
		camPos += d * camRight;
	};
};

