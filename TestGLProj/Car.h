#pragma once
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform2.hpp>
#include <stdlib.h>
#include <stdio.h>


#include <string>
#include <iostream>

using namespace std;

class Car
{
private:


	glm::vec3 carPosition;	// car position
	
	


	/* Functions */
	float triangleWave(float move);
	void updateSpeed(float vel, float dt);
	void updateAngle(float dt);
	void resetPosition(void);
public:
	// General
	float old_t = 0;		// previous time tracker for dt
	float motionScaler = 2;	// scales speed
	float _3pDistance = 10.0f;	// Camera distance from car
	float carScale = 1.0f;

	// Steering values
	float modelRotAngle = 0;
	float curRotAngle = 0;	// character directional angle
	float maxRotAngle = 45;	// max turn angle
	float turnRadius = 0;
	float carLength = 1;	// crucial for get rotation radius

	// Movement values
	float posX = 0;			// character x position
	float posZ = 0;			// character z position
	float speed = 0;		// character speed
	float maxSpeed = 9.0;	// character speed
	float accel = 1.0;		// car acceleration
	glm::mat4 oldMat;		// Old position

	/* Use all bools to have proper multikey controls */
	// Controls
	bool isBreaking = false;	// Toggles fast decel
	float speedDir = 0.0f;		// Ranges from -1.0 to 1.0
	float turnDir = 0.0f;		// Ranges from -1.0 to 1.0

	bool carCollided = false;


	float toRad(float degree);
	void CarControls(void);
	void CarKeyDown(unsigned char key);
	void CarKeyUp(unsigned char key);
	glm::vec3 GetCarPosition();
	glm::mat4 SetCarModelMatrix(glm::mat4 *oldModel, float *angle);
};