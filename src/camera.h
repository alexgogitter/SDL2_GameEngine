#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>

//#include <vector>

// Enum class declared instead of enum as other enum types can be seen as equivilent
enum class Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN
};


// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.5f;
const float ZOOM = 45.0f;

class Camera {
public:
	// Camera attributes
	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;
	// euler angles
	float Yaw;
	float Pitch;
	// camera options
	float MovementSpeed;
	float MouseSensitivity;
	float Zoom;

	// std::string GetPositionCoords(void) {
	// 	std::stringstream s;
	// 	s << "[" << Position.x << "," << Position.y << "," << Position.z << "]";
	// 	return s.str();
	// }

	// Constructor init list is used as it can define them based on the constructor used and putting the 
	// assigmnets in the constructor body is twice the work
	// Constructor with vectors
	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
	{
		Position = position;
		WorldUp = up;
		Yaw = yaw;
		Pitch = pitch;
		updateCameraVectors();
	}

	// constructor with scalar values
	Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
	{
		Position = glm::vec3(posX, posY, posZ);
		WorldUp = glm::vec3(upX, upY, upZ);
		Yaw = yaw;
		Pitch = pitch;
		updateCameraVectors();
	}

	~Camera() { }


	// returns the view matrix calculated using Euler Angles and the LookAt Matrix
	glm::mat4 GetViewMatrix()
	{
		return glm::lookAt(Position, Position + Front, Up);
	}

	// Process keyboard input // Speed multiplier for movemnt speed increase with shift key
	void processKeyboard(Camera_Movement direction, float deltaTime, float speedMultiplier = 1) {

		// Camera will move at 2.5 units per second
		float velocity = MovementSpeed * deltaTime;

		if (direction == Camera_Movement::FORWARD)
			Position += Front * velocity * speedMultiplier;
		if (direction == Camera_Movement::BACKWARD)
			Position -= Front * velocity * speedMultiplier;
		if (direction == Camera_Movement::LEFT)
			Position -= Right * velocity * speedMultiplier;
		if (direction == Camera_Movement::RIGHT)
			Position += Right * velocity * speedMultiplier;

		// Space and ctrl for up and down
		if (direction == Camera_Movement::UP)
			Position += Up * velocity;
		if (direction == Camera_Movement::DOWN)
			Position -= Up * velocity;
	}

	// processes input received from a mouse input system. Expects the offset value in both the x and y direction.
	void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
	{
		xoffset *= MouseSensitivity;
		yoffset *= MouseSensitivity;

		Yaw += xoffset;
		Pitch += yoffset;

		// make sure that when pitch is out of bounds, screen doesn't get flipped
		if (constrainPitch)
		{
			if (Pitch > 89.0f)
				Pitch = 89.0f;
			if (Pitch < -89.0f)
				Pitch = -89.0f;
		}

		// update Front, Right and Up Vectors using the updated Euler angles
		updateCameraVectors();
	}

	// processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
	void ProcessMouseScroll(float yoffset)
	{
		Zoom -= yoffset;
		if (Zoom < 1.0f)
			Zoom = 1.0f;
		if (Zoom > 45.0f)
			Zoom = 45.0f;
	}


private:

	// calculates the front vector from the Camera's (updated) Euler Angles
	void updateCameraVectors() {
		// calculate the new front vector
		Front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		Front.y = sin(glm::radians(Pitch));
		Front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		Front = glm::normalize(Front);
		// also re-calculate the Right and Up vector
		Right = glm::normalize(glm::cross(Front, WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		Up = glm::normalize(glm::cross(Right, Front));
	}
};

#endif // CAMERA_H