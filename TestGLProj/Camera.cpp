#include "Camera.h"

const float STEP = 0.1f;


Camera::Camera(glm::vec3 e, glm::vec3 c, glm::vec3 u)
{
	eye = e;
	center = c;
	up = u;
	forward = glm::vec3(0.f, 0.f, 1.f);
	position = glm::vec3(0.f, 3.f, -10.f);
}


glm::vec3 Camera::getEye()
{
	return eye;
}


glm::vec3 Camera::getCenter()
{
	return center;
}


glm::vec3 Camera::getUp()
{
	return up;
}

void Camera::setLookAt(glm::mat4 *view)
{
	*view = glm::lookAt(position, position + forward, up);
}

void Camera::setPosition(glm::vec3 pos)
{
	eye = pos;
}

void Camera::changeTarget(glm::vec3 target)
{

	center = target;// +glm::normalize;
	forward = glm::normalize(center + eye);
}

void Camera::applyTranslation(glm::vec3 trans)
{
	center += trans;
}

void Camera::specialControls(int key, int x, int y)
{
	const float delta = 0.5f;

	glm::vec3 direction;


	switch (key) {
	case GLUT_KEY_UP: // Arrow key up
		position += 0.5f * forward;
		break;
	case GLUT_KEY_DOWN: // Arrow key down
		position -= 0.5f * forward;
		break;
	case GLUT_KEY_LEFT: // Arrow key left
		position -= glm::normalize(glm::cross(forward, up)) * delta;
		break;
	case GLUT_KEY_RIGHT: // Arrow key right
		position += glm::normalize(glm::cross(forward, up)) * delta;
		break;
	case 27: // this is an ascii value
		exit(0);
		break;
	}
	
}

void Camera::normalControls(int key, int x, int y)
{
	glm::vec3 direction;


	switch (key) {
	case 'w':
		//view *= glm::rotate(2.f, glm::vec3(0.f, 1.f, 0.f));
		position += 1.f * forward;
		break;
	case 's':
		position -= 1.f * forward;
		break;
	case 'a':
		yaw -= 2;
		direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		direction.y = sin(glm::radians(pitch));
		direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		forward = glm::normalize(direction);
		break;
	case 'd':
		yaw += 2;
		direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		direction.y = sin(glm::radians(pitch));
		direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		forward = glm::normalize(direction);
		break;
	case 27: // this is an ascii value
		exit(0);
		break;
	}

}