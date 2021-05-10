#include "Camera.h"


void Camera::resetFlycam(void)
{
	verCam = 0;
	horCam = 0;
	camX = 0;
	camY = 0;
	camZ = -20;
}

void Camera::OnToggleFlycam(Car car)
{
	camX = -car.posX - flyCamResetOffset * sin(car.toRad(car.curRotAngle));
	camZ = -car.posZ - flyCamResetOffset * cos(car.toRad(car.curRotAngle));
	horCam = -car.curRotAngle;
}

void Camera::FlycamControls(Car car)
{
	horCam += camAngSpeedModifier * camHorDir;
	verCam += camAngSpeedModifier * camVerDir;
	camX -= camSpeedModifier * camSpeedDir * sin(car.toRad(horCam));
	camY += camSpeedModifier * camSpeedDir * sin(car.toRad(verCam));
	camZ += camSpeedModifier * camSpeedDir * cos(car.toRad(horCam));
}


glm::mat4 Camera::SetViewMatrix(Car car)
{
	glm::mat4 view;

	float swayScaler = 0.5f;																// Swing modifier (Range: 0.0 to 1.0)
	float speedMod = 1 + swayScaler * pow(car.speed / car.maxSpeed, 2);						// Camera fly back value
	if (car.speed <= 1.0f) speedMod = 1.0f;	// Bottom cap car.speed at 1.0f
	float smoothCamTurn = 0.1f * car.curRotAngle * sqrt(abs(car.speed / car.maxSpeed));		// Camera side swing value

	// Calculate Camera Position
	cameraPosition = car.GetCarPosition() + glm::vec3(
		speedMod * car._3pDistance * sinf(car.toRad(car.modelRotAngle + smoothCamTurn)),	// x camera position
		speedMod * car._3pDistance * 0.25f,													// y camera position
		speedMod * car._3pDistance * cosf(car.toRad(car.modelRotAngle + smoothCamTurn)));	// z camera position

	if (flyCamMode) /// Fly camera
	{
		camPosition = glm::translate(camX, camY, camZ);
		//camPosition = glm::translate(0.f, -10.f, -200.f);
		view = glm::rotate((float)verCam, 1.0f, 0.0f, 0.0f);	// Vertical camera movement - PITCH
		//view = glm::rotate(0.f, 1.0f, 0.0f, 0.0f);
		view *= glm::rotate((float)horCam, 0.0f, 1.0f, 0.0f);	// Horizontal camera movement - YAW
		view *= camPosition;									// Camera position
	}
	else /// Follow camera
	{
		view = glm::lookAt(cameraPosition, car.GetCarPosition(), glm::vec3(0, 1, 0));
	}
	return view;
}




void Camera::CameraKeyDown(unsigned char key, Car car)	// On key down
{
	switch (key) {
		/// FLYCAMERA CONTROLS ///
	case 'f':	// camera move forward
		camSpeedDir = 1;
		break;
	case 'v':	// camera move backward
		camSpeedDir = -1;
		break;
	case 'h':	// Reset Fly Cam
		flyCamMode = true;
		resetFlycam();
		break;
	case 'c':	// Toggle fly camera and follow
		flyCamMode = !flyCamMode;
		OnToggleFlycam(car);
		break;
	}
}

void Camera::CameraKeyUp(unsigned char key)	// On key down
{
	switch (key) {
		/// FLYCAMERA CONTROLS ///
	case 'f':	// camera move forward
		camSpeedDir = 0;
		break;
	case 'v':	// camera move backward
		camSpeedDir = 0;
		break;
	}
}

void Camera::CameraSpecialInputDown(int key)
{
	switch (key)
	{
	case GLUT_KEY_UP:
		camVerDir = -1.0f;
		break;
	case GLUT_KEY_DOWN:
		camVerDir = 1.0f;
		break;
	case GLUT_KEY_LEFT:
		camHorDir = -1.0f;
		break;
	case GLUT_KEY_RIGHT:
		camHorDir = 1.0f;
		break;
	}
}

void Camera::CameraSpecialInputUp(int key)
{
	switch (key)
	{
	case GLUT_KEY_UP:
		camVerDir = 0;
		break;
	case GLUT_KEY_DOWN:
		camVerDir = 0;
		break;
	case GLUT_KEY_LEFT:
		camHorDir = 0;
		break;
	case GLUT_KEY_RIGHT:
		camHorDir = 0;
		break;
	}
}
