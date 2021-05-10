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
	void flipDirection();
	void onCarCollision();

public:
	// General
	float M_PI = 3.14159265359;		// Pi
	float old_t = 0;				// Time: Previous time tracker for dt
	float motionScaler = 2;			// Scaler: Arbitrary value that scales speed
	float _3pDistance = 10.0f;		// Distance: Camera distance from car in follow mode
	float carScale = 1.0f;
	
	// Steering values
	float turnRate = 60.0f;			// Anglular Velocity: Rate at which imaginary steering wheel is turned
	float modelRotAngle = -58;		// Angle: Rotation of car model with respect to world coordinates (degrees)
	float curRotAngle = 0;			// Angle: Rotation of front wheels with respect to car model (degrees)
	float maxRotAngle = 45;			// Angle: Max turn angle (degrees)
	float turnRadius = 0;			// Length: Ackermann turning radius
	float carLength = 1;			// Length: Crucial for get rotation radius

	// Movement values
	float posX = 0;					// Position: Car x position relative to world
	float posZ = -70;					// Position: Car z position relative to world
	float speed = 0;				// Velocity: Car directional speed relative to model
	float maxSpeed = 9.0;			// Speed: Arbitrary maximum speed
	float accel = 3.0;				// Acceleration: Arbitrary car acceleration
	float breakAccel = -10.0f;		// Acceleration: Arbitrary break deceleration value
	float slowDecel = -1.0f;		// Acceleration: Arbitrary rolling halt deceleration

	/* Use all bools to have proper multikey controls */
	// Controls
	bool isBreaking = false;				// Toggles fast decel
	bool isAcceleratingForward = false;		// isAcceleratingForward
	bool isAcceleratingBackward = false;	// isAcceleratingBackward
	bool isTurningLeft = false;				// isSteeringLeft
	bool isTurningRight = false;			// isSteeringRight

	float speedDir = 0.0f;					// UnitVector: Ranges from -1.0 to 1.0
	float turnDir = 0.0f;					// UnitVector: Ranges from -1.0 to 1.0

	bool carCollided = false;


	float toRad(float degree);
	void CarControls(void);
	void CarKeyDown(unsigned char key);
	void CarKeyUp(unsigned char key);
	glm::vec3 GetCarPosition();
	glm::mat4 SetCarModelMatrix();
};