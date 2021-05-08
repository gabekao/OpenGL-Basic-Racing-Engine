#include "Car.h"

double M_PI = 3.14159265359;

/* returns radians from degrees */
float Car::toRad(float degree)
{
	double pi = 3.14159265359;
	return (degree * (pi / 180));
}

// psuedo drag force
void Car::updateSpeed(float vel, float dt) 
{
	/*speed += dt * (isBreaking ? 3.0f * -vel :	// Breaks; fast deceleration
		isAcceleratingForward ? accel :			// Apply forward force
		isAcceleratingBackward ? -accel :		// Apply reverse force
		(-0.2 > vel || vel > 0.2) ? -vel :		// Set minimum speed
		0.5f * -vel);							// Drag; slow deceleration
		*/
	float dir = abs(speed) / speed;
	float breakAccel = -30.0f;
	float slowDecel = -1.0f;
	
	if (carCollided)
	{
		flipDirection();
		speed = -speed;
	}
	speed += dt * (isBreaking ? dir * breakAccel :	// Breaks; fast deceleration
		speedDir != 0 ? speedDir * accel :			// Apply acceleration WRT direction
		(-0.5 < vel && vel < 0.5) ? (-vel / dt) :	// Set minimum speed
		dir * slowDecel);							// Rolling stop; slow deceleration
	/*
	Every frame (0.0069444.. seconds):
		speed = (1 - 3/144) * speed
	*/
}

void Car::flipDirection()
{
	if (speedDir < 0)
		speedDir = 1;
	else
		speedDir = -1;
}

// psuedo drag force
void Car::updateAngle(float dt) 
{
	curRotAngle += turnDir != 0 ? dt * turnDir * 180.0f * cosf(toRad(curRotAngle)) :	// Turn Wheels
		1.0 > curRotAngle && -1.0 < curRotAngle ? -curRotAngle :						// Set Straight
		-dt * 5.0f * curRotAngle;														// Auto Straighten wheels

	if (curRotAngle > maxRotAngle) curRotAngle = maxRotAngle;		// Set max angle
	if (curRotAngle < -maxRotAngle) curRotAngle = -maxRotAngle;		// Set min angle
}

/* passing step counter creates a triangle wave output
   move is an incremental value scaled by rate
   output values oscilates between -max and +max */
float Car::triangleWave(float move)
{
	int max = 30; // degrees positive/negative
	int rate = 3; // better if factor of max
	return max - 2 * abs(abs(rate * (int)move + max / 2) % (2 * max) - max);
}

void Car::CarControls(void)
{
	// Scale kinematics
	carLength = (1.45 + 1.55) * carScale; // front wheel center offset + back wheels center offset

	float t = glutGet(GLUT_ELAPSED_TIME);		// -> Get new time
	float dt = (t - (float)old_t) / 1000.0f;	// -> Get change in time
	old_t = t;									// -> Set new time to old time

	updateSpeed(speed, dt);
	updateAngle(dt);

	turnRadius = carLength / tanf(toRad(curRotAngle));

	modelRotAngle += dt * 180.0f * speed / (M_PI * turnRadius);
	posX -= dt * (motionScaler)*speed * sin(toRad(modelRotAngle));
	posZ -= dt * (motionScaler)*speed * cos(toRad(modelRotAngle));
	

	float r = ((float)rand() / (RAND_MAX)) + 1;	// Max/min padding
	if (speed > maxSpeed)						// Set max speed
		speed = maxSpeed;// -r;
	if (speed < -0.5 * maxSpeed)				// Set min speed
		speed = -0.5 * (maxSpeed - r);
	
	string s = (isBreaking ? "[Breaking]" : speedDir > 0 ? "[Gas]" : speedDir < 0 ? "[Reverse]" : "");
	//cout << speed << "mph \t" << s << endl;
	//cout << speed << endl;
}


glm::mat4 Car::SetCarModelMatrix()
{
	glm::mat4 mat;

	// Calculate Model Position
	carPosition = glm::vec3(posX, -4.275, posZ);
	mat = glm::translate(carPosition) * glm::rotate(modelRotAngle, 0.0f, 1.0f, 0.0f);

	if (modelRotAngle > 360)
		modelRotAngle -= 360;
	if (modelRotAngle < 0)
		modelRotAngle += 360;

	return mat;
}


glm::vec3 Car::GetCarPosition()
{
	return carPosition;
}


void Car::CarKeyDown(unsigned char key)
{
	switch (key) {
		/// MODEL CONTROLS ///
	case 'a':	// turn left
		turnDir = 1;
		break;
	case 'd':	// turn right
		turnDir = -1;
		break;
	case 'w':	// accel forward
		speedDir = 1;
		break;
	case 's':	// accel backward
		speedDir = -1;
		break;
	case 'b':	// breaks
		isBreaking = true;
		speedDir = 0;
		break;
	case 'r':	// Reset Model Position
		resetPosition();
		break;
	}
}

void Car::CarKeyUp(unsigned char key)
{
	switch (key) {
		/// MODEL CONTROLS ///
	case 'a':	// turn left
		turnDir = 0;
		break;
	case 'd':	// turn right
		turnDir = 0;
		break;
	case 'w':	// accel forward
		speedDir = 0;
		break;
	case 's':	// accel backward
		speedDir = 0;
		break;
	case 'b':	// breaks
		isBreaking = false;
		break;
	}
}

void Car::resetPosition(void)
{
	curRotAngle = 0;
	posX = 0;
	posZ = 0;
	speed = 0;
}
