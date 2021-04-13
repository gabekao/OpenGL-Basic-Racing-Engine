#include "Camera.h"

const float STEP = 0.1f;


Camera::Camera(glm::vec3 e, glm::vec3 c, glm::vec3 u)
{
	eye = e;
	center = c;
	up = u;
	forward = glm::vec3(0.f, 0.f, 1.f);
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
	*view = glm::lookAt(eye, center, up);
}

void Camera::setPosition(glm::vec3 pos)
{
	eye = pos;
}

void Camera::changeTarget(glm::vec3 target)
{
	center = target;
	forward = glm::normalize(center + eye);
}


void Camera::specialControls(int key, int x, int y)
{
	switch (key) {
	case GLUT_KEY_UP: // Arrow key up
		eye += glm::vec3(0.f, 1.f, 0.f) * STEP;
		break;
	case GLUT_KEY_DOWN: // Arrow key down
		eye -= glm::vec3(0.f, 1.f, 0.f) * STEP;
		break;
	//case GLUT_KEY_LEFT: // Arrow key left
		// Rotate center vector to the left based on horizontal delta
		//eye = glm::mat3(glm::rotate(delta, up)) * eye;
		//break;
	//case GLUT_KEY_RIGHT: // Arrow key right
		// Rotate center vector to the right based on horizontal delta
		//eye = glm::mat3(glm::rotate(-delta, up)) * eye;
		//break;
	case 27: // this is an ascii value
		exit(0);
		break;
	}
	
}

void Camera::normalControls(int key, int x, int y)
{
	switch (key) {
	case 'f':
		eye -= glm::normalize(center + eye) * STEP;
		break;
	case 'v':
		eye += glm::normalize(center + eye) * STEP;
		break;
	case 27: // this is an ascii value
		exit(0);
		break;
	}

}