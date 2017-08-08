#pragma once

// Std. Includes
#include <vector>

// GL Includes
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>



// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	BOTTOM,
	UP
};

// Default camera values
const GLfloat YAW = -90.0f;
const GLfloat PITCH = 0.0f;
const GLfloat SPEED = 25000.0f; // mm
const GLfloat SENSITIVTY = 0.05f; // degree per pixel
const GLfloat ZOOM = 76.80095294072474; // degree
const GLfloat FARAWAY = 4000000.0f;
//>> > height = 2832; p = 35.8 / 4240.0; f = 15
//>> > 2 * degrees(atan(height / 2 * p / f))
// An abstract camera class that processes input and calculates the corresponding Eular Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
	// Camera Attributes
	glm::vec3 Position;
	glm::vec3 Front; 
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;
	// Eular Angles
	GLfloat Yaw;
	GLfloat Pitch;
	// Camera options
	GLfloat MovementSpeed;
	GLfloat MouseSensitivity;
	GLfloat Zoom;

	// Constructor with vectors
	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), GLfloat yaw = YAW, GLfloat pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVTY), Zoom(ZOOM)
	{
		this->Position = position;
		this->WorldUp = up;
		this->Yaw = yaw;
		this->Pitch = pitch;
		this->updateCameraVectors();
	}
	// Constructor with scalar values
	Camera(GLfloat posX, GLfloat posY, GLfloat posZ, GLfloat upX, GLfloat upY, GLfloat upZ, GLfloat yaw, GLfloat pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVTY), Zoom(ZOOM)
	{
		this->Position = glm::vec3(posX, posY, posZ);
		this->WorldUp = glm::vec3(upX, upY, upZ);
		this->Yaw = yaw;
		this->Pitch = pitch;
		this->updateCameraVectors();
	}

	// Returns the view matrix calculated using Eular Angles and the LookAt Matrix
	glm::mat4 GetViewMatrix()
	{
		return glm::lookAt(this->Position, this->Position + this->Front, this->Up);
	}

	// Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
	void ProcessKeyboard(Camera_Movement direction, GLfloat deltaTime)
	{
		GLfloat velocity = this->MovementSpeed * deltaTime;

		glm::vec3 ground_front = this->Front;
		ground_front.y = 0.0f;
		ground_front = glm::normalize(ground_front);

		glm::vec3 ground_right = this->Right;
		ground_right.y = 0.0f;
		ground_right = glm::normalize(ground_right);

		if (direction == FORWARD)
			this->Position += ground_front * velocity;
		if (direction == BACKWARD)
			this->Position -= ground_front * velocity;
		if (direction == LEFT)
			this->Position -= ground_right * velocity;
		if (direction == RIGHT)
			this->Position += ground_right * velocity;
		if (direction == UP)
			this->Position += this->Up * velocity;
		if (direction == BOTTOM)
			this->Position -= this->Up * velocity;
	}

	// Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
	void ProcessMouseMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constrainPitch = true)
	{
		xoffset *= this->MouseSensitivity;
		yoffset *= this->MouseSensitivity;

		this->Yaw += xoffset;
		this->Pitch += yoffset;

		// Make sure that when pitch is out of bounds, screen doesn't get flipped
		if (constrainPitch)
		{
			if (this->Pitch > 89.0f)
				this->Pitch = 89.0f;
			if (this->Pitch < -89.0f)
				this->Pitch = -89.0f;
		}

		// Update Front, Right and Up Vectors using the updated Eular angles
		this->updateCameraVectors();
	}

	// Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
	void ProcessMouseScroll(GLfloat yoffset)
	{
		//if (this->Zoom >= 1.0f && this->Zoom <= 85.0f)
		//	this->Zoom -= yoffset;
		//if (this->Zoom <= 1.0f)
		//	this->Zoom = 1.0f;
		//if (this->Zoom >= 85.0f)
		//	this->Zoom = 85.0f;
		this->Position += this->Front * (yoffset * this->MovementSpeed);
	}
	void SetPosition(glm::vec3 pos)
	{
		this->Position = pos;
		this->updateCameraVectors();
	}
	void ViewSwitch(int pos)
	{
		glm::vec3 camera_pos[6] = {
			//glm::vec3(0.0f,  0.0f,  FARAWAY),
			//glm::vec3(FARAWAY,  0.0f,  -80.0f),
			//glm::vec3(0.0f,  0.0f, -FARAWAY),
			//glm::vec3(-FARAWAY, 0.0f,  -80.0f),
			//glm::vec3(0.0f,  FARAWAY,  -80.0f),
			//glm::vec3(0.0f, -FARAWAY,  -80.0f)
			glm::vec3(0.0f,  0.0f,  FARAWAY),
			glm::vec3(FARAWAY,  0.0f,  0.0f),
			glm::vec3(0.0f,  0.0f, -FARAWAY),
			glm::vec3(-FARAWAY, 0.0f,  0.0f),
			glm::vec3(0.0f,  FARAWAY,  0.0f),
			glm::vec3(0.0f, -FARAWAY,  0.0f)
		};
		GLfloat camera_yaw[6] = {
			-90, -180, -270, -360, -270, -270
		};
		GLfloat camera_pitch[6] = {
			0, 0, 0, 0, -90, 90
		};
		glm::vec3 world_ups[6] = {
			//glm::vec3(-1.0f,  0.0f,  0.0f),
			//glm::vec3(0.0f,  0.0f,  1.0f),
			//glm::vec3(-1.0f,  0.0f,  0.0f),
			//glm::vec3(0.0f,  0.0f,  1.0f),
			//glm::vec3(0.0f,  0.0f,  1.0f),
			//glm::vec3(0.0f,  0.0f,  1.0f)
			glm::vec3(0.0f,  1.0f,  0.0f),
			glm::vec3(0.0f,  1.0f,  0.0f),
			glm::vec3(0.0f,  1.0f,  0.0f),
			glm::vec3(0.0f,  1.0f,  0.0f),
			glm::vec3(0.0f,  0.0f,  -1.0f),
			glm::vec3(0.0f,  0.0f,  -1.0f)
		};
		this->Position = camera_pos[pos];
		this->Yaw = camera_yaw[pos];
		this->Pitch = camera_pitch[pos];
		this->WorldUp = world_ups[pos];
		this->Zoom = ZOOM;
		this->updateCameraVectors(); 
	}
private:
	// Calculates the front vector from the Camera's (updated) Eular Angles
	void updateCameraVectors()
	{
		// Calculate the new Front vector
		glm::vec3 front;
		front.x = cos(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));
		front.y = sin(glm::radians(this->Pitch));
		front.z = sin(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));
		this->Front = glm::normalize(front);
		// Also re-calculate the Right and Up vector
		this->Right = glm::normalize(glm::cross(this->Front, this->WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		this->Up = glm::normalize(glm::cross(this->Right, this->Front));
	}
};