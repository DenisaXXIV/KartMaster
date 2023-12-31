﻿#include "camera.h"

#include <GL/glew.h>

#include <GLM.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <iostream>

#include <glfw3.h>

Camera::Camera(const int width, const int height, const glm::vec3& position)
{
	startPosition = position;
	Set(width, height, position);
}

void Camera::Set(const int width, const int height, const glm::vec3& position)
{
	this->isPerspective = true;
	this->yaw = YAW;
	this->pitch = PITCH;

	this->zoom = FOV;
	this->width = width;
	this->height = height;
	this->zNear = zNEAR;
	this->zFar = zFAR;

	this->worldUp = glm::vec3(0, 1, 0);
	this->position = position;
	this->position.y = -0.35f;



	lastX = width / 2.0f;
	lastY = height / 2.0f;
	bFirstMouseMove = true;

	UpdateCameraVectors();
}

void Camera::Reset(const int width, const int height)
{
	Set(width, height, startPosition);
}

void Camera::Reshape(int windowWidth, int windowHeight)
{
	width = windowWidth;
	height = windowHeight;

	// define the viewport transformation
	glViewport(0, 0, windowWidth, windowHeight);
}

const glm::vec3 Camera::GetPosition() const
{
	return position;
}

const glm::mat4 Camera::GetViewMatrix() const
{
	// Returns the View Matrix
	return glm::lookAt(position, position + forward, up);
}

const glm::mat4 Camera::GetProjectionMatrix() const
{
	glm::mat4 Proj = glm::mat4(1);
	if (isPerspective) {
		float aspectRatio = ((float)(width)) / height;
		Proj = glm::perspective(glm::radians(zoom), aspectRatio, zNear, zFar);
	}
	else {
		float scaleFactor = 2000.f;
		Proj = glm::ortho<float>(
			-width / scaleFactor, width / scaleFactor,
			-height / scaleFactor, height / scaleFactor, -zFar, zFar);
	}
	return Proj;
}

void Camera::ProcessKeyboard(ECameraMovementType direction, float deltaTime)
{
	float velocity = (float)(cameraSpeedFactor * deltaTime);
	glm::vec3 horizontalForward = glm::normalize(glm::vec3(forward.x, 0.0f, forward.z));
	float rotationAngle = 0.0f;

	switch (direction) {
	case ECameraMovementType::FORWARD:
		position += horizontalForward * velocity;
		break;
	case ECameraMovementType::BACKWARD:
		position -= horizontalForward * velocity;
		break;
	case ECameraMovementType::LEFT:
		position -= right * velocity;
		rotationAngle = glm::radians(-90.0f);
		break;
	case ECameraMovementType::RIGHT:
		position += right * velocity;
		rotationAngle = glm::radians(90.0f);
		break;
	case ECameraMovementType::UP:
		position.y += velocity;
		break;
	case ECameraMovementType::DOWN:
		position.y -= velocity;
		break;
	}

	// Rotate the camera
	if (rotationAngle != 0.0f) {
		yaw += rotationAngle;
		UpdateCameraVectors();
	}
}

void Camera::MouseControl(float xPos, float yPos)
{
	if (bFirstMouseMove) {
		lastX = xPos;
		lastY = yPos;
		bFirstMouseMove = false;
	}

	return;

	float xChange = xPos - lastX;
	float yChange = lastY - yPos;
	lastX = xPos;
	lastY = yPos;

	if (fabs(xChange) <= 1e-6 && fabs(yChange) <= 1e-6) {
		return;
	}
	xChange *= mouseSensitivity;
	yChange *= mouseSensitivity;

	ProcessMouseMovement(xChange, yChange);
}

void Camera::ProcessMouseScroll(float yOffset)
{
	if (zoom >= 1.0f && zoom <= 90.0f) {
		zoom -= yOffset;
	}
	if (zoom <= 1.0f)
		zoom = 1.0f;
	if (zoom >= 90.0f)
		zoom = 90.0f;
}
void Camera::ProcessMouseMovement(float xOffset, float yOffset, bool constrainPitch)
{
	yaw += xOffset;
	pitch += yOffset;

	//std::cout << "yaw = " << yaw << std::endl;
	//std::cout << "pitch = " << pitch << std::endl;

	// Avem grijã sã nu ne dãm peste cap
	if (constrainPitch) {
		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;
	}

	// Se modificã vectorii camerei pe baza unghiurilor Euler
	UpdateCameraVectors();
}

void Camera::UpdateCameraVectors()
{
	// Calculate the new forward vector
	this->forward.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	this->forward.y = sin(glm::radians(pitch));
	this->forward.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	this->forward = glm::normalize(this->forward);

	// Also re-calculate the Right and Up vector
	right = glm::normalize(glm::cross(forward, worldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
	up = glm::normalize(glm::cross(right, forward));
}