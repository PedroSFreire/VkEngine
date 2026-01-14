#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../../Renderer/Platform/Window.h"

class Camera
{
	

public:
	Camera() = default;
	~Camera() = default;
	Camera(const Camera&) = delete;
	Camera(Camera&& other) noexcept {
		camPos = other.camPos;
		camDirection = other.camDirection;
		camUp = other.camUp;
		camRight = other.camRight;
		lastMouseX = other.lastMouseX;
		lastMouseY = other.lastMouseY;
		sensitivity = other.sensitivity;
		camSensivity = other.camSensivity;
		firstMouse = other.firstMouse;
		pitch = other.pitch;
		yaw = other.yaw;
	}

	Camera& operator=(Camera&& other) noexcept {
		if (this != &other) {
			camPos = other.camPos;
			camDirection = other.camDirection;
			camUp = other.camUp;
			camRight = other.camRight;
			lastMouseX = other.lastMouseX;
			lastMouseY = other.lastMouseY;
			sensitivity = other.sensitivity;
			camSensivity = other.camSensivity;
			firstMouse = other.firstMouse;
			pitch = other.pitch;
			yaw = other.yaw;
		}
		return *this;
	}


	void updateCam(double mouseX, double mouseY);

	// Getters
	glm::vec3& getPos() { return camPos; };
	glm::vec3& getDirection() { return camDirection; }
	glm::vec3& getUp() { return camUp; }
	float getFov() const{ return fov; }

	// Movement
	//void goForward(float d) { camPos += d * camDirection; }
	//void goUp(float d) { camPos += d * camUp; };
	//void goRight(float d) { camPos += d * camRight; };
	void processInput(float deltaTime, Window& window);

private:

	void goForward(float d) { camPos += d * camDirection; }
	void goUp(float d) { camPos += d * camUp; };
	void goRight(float d) { camPos += d * camRight; };

	glm::vec3 camPos = glm::vec3(2.0f, 2.0f, 2.0f);
	glm::vec3 camDirection = glm::normalize(glm::vec3(0.0f, 0.0f, 0.0f) - camPos);
	glm::vec3 camUp = glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec3 camRight = glm::normalize(glm::cross(camDirection, camUp));

	float fov = 60;
	double lastMouseX = 0;
	double lastMouseY = 0;
	float sensitivity = 0.001f;
	float camSensivity = 0.1f;
	bool firstMouse = true;

	float pitch = -30;
	float yaw = -135;
};

