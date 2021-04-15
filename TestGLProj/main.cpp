#include <GL/glew.h>
#include <GL/freeglut.h>

//glm library
#include <glm/glm.hpp>
#include <glm/gtx/transform2.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Model.h"
#include "Shader.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <math.h>

using namespace std;

Shader shader; // loads our vertex and fragment shaders

/* ROBOT PARTS START */
/*
Model* torso;		//a cylinder torso [parent object]
Model* head;		// child of torso
Model* limb;		// child of torso
Model* ball;
*/

/* MODELS */
/*
Model* tree;
Model* coilthing;
Model* trafficcone;
Model* wheel;
Model* flashlight;
*/
Model* car;
Model* box;
Model* plane;
Model* wheel;

glm::mat4 limbscale;		// char limb scaling matrix

glm::mat4 rotationRight;	// char right limbs rotation matrix
glm::mat4 rotationLeft;		// char left limbs rotation matrix
glm::mat4 offset;			// char limbs pivot point offset matrix

glm::mat4 camPosition;		// flycamera position matrix

glm::mat4 projection;		// projection matrix
glm::mat4 view;				// where the camera is lookin
glm::mat4 model;			// where the model (i.e., the myModel) is located wrt the camera

glm::vec3 cameraPosition;	// camera pos
glm::vec3 modelPosition;	// model posi

/*
float steps = 0;	// character step counter for walking animation
float jointA;		// character rotation angle of limbs
*/

// Flycam values
float verCam = 0;		// flycamera vertical angle
float horCam = 0;		// flycamera horizontal angle
float camX = 0;		// flycamera x position
float camY = 0;		// flycamera y position
float camZ = -20;	// flycamera z position
float flyCamResetOffset = 7.5;	// flycam magnititudal offset distance behind player after mode toggled
float camSpeedModifier = 0.25f;
float camAngSpeedModifier = 1.0f;

// General
float old_t = 0;		// previous time tracker for dt
float motionScaler = 2;	// scales speed
float _3pDistance = 10.0f;	// Camera distance from car
float carScale = 1.0f;

// Movement values
float posX = 0;			// character x position
float posZ = 0;			// character z position
float speed = 0;		// character speed
float maxSpeed = 60;	// character speed
float accel = 10;		// car acceleration

// Steering values
float modelRotAngle = 0;
float curRotAngle = 0;	// character directional angle
float maxRotAngle = 45;	// max turn angle
float turnRadius = 0;
float carLength = 1;	// crucial for get rotation radius

/* Use all bools to have proper multikey controls */
// Controls
bool flyCamMode = false;	// [first person / flycamera] toggle
bool isBreaking = false;	// Toggles fast decel
float speedDir = 0.0f;		// Ranges from -1.0 to 1.0
float turnDir = 0.0f;		// Ranges from -1.0 to 1.0

float camSpeedDir = 0.0f;	// Ranges from -1.0 to 1.0
float camVerDir = 0.0f;		// Ranges from -1.0 to 1.0
float camHorDir = 0.0f;		// Ranges from -1.0 to 1.0

/* returns radians from degrees */
float toRad(float degree)
{
	double pi = 3.14159265359;
	return (degree * (pi / 180));
}

void updateSpeed(float vel, float dt) // psuedo drag force
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
	speed += dt * (isBreaking ? dir * breakAccel :	// Breaks; fast deceleration
		speedDir != 0 ? speedDir * accel :			// Apply acceleration WRT direction
		(-0.5 < vel && vel < 0.5) ? (-vel / dt) :	// Set minimum speed
		dir * slowDecel);							// Rolling stop; slow deceleration
	/*
	Every frame (0.0069444.. seconds):
		speed = (1 - 3/144) * speed
	*/
}

void updateAngle(float dt) // psuedo drag force
{
	curRotAngle += turnDir != 0 ? dt * turnDir * 180.0f * cosf(toRad(curRotAngle)) :	// Turn Wheels
		1.0 > curRotAngle && -1.0 < curRotAngle ? -curRotAngle :						// Set Straight
		-dt * 5.0f * curRotAngle;														// Auto Straighten wheels

	if (curRotAngle > maxRotAngle) curRotAngle = maxRotAngle;		// Set max angle
	if (curRotAngle < -maxRotAngle) curRotAngle = -maxRotAngle;		// Set min angle
}

void resetFlycam(void)
{
	verCam = 0;
	horCam = 0;
	camX = 0;
	camY = 0;
	camZ = -20;
}

void resetPosition(void)
{
	curRotAngle = 0;
	posX = 0;
	posZ = 0;
	speed = 0;
}

/* passing step counter creates a triangle wave output
   move is an incremental value scaled by rate
   output values oscilates between -max and +max */
float triangleWave(float move)
{
	int max = 30; // degrees positive/negative
	int rate = 3; // better if factor of max
	return max - 2 * abs(abs(rate * (int)move + max / 2) % (2 * max) - max);
}

/* report GL errors, if any, to stderr */
void checkError(const char* functionName)
{
	GLenum error;
	while ((error = glGetError()) != GL_NO_ERROR) {
		std::cerr << "GL error " << error << " detected in " << functionName << std::endl;
	}
}

void initShader(void)
{
	shader.InitializeFromFile("shaders/phong.vert", "shaders/phong.frag");
	shader.AddAttribute("vertexPosition");
	shader.AddAttribute("vertexNormal");

	checkError("initShader");
}

void initRendering(void)
{
	glClearColor(0.117f, 0.565f, 1.0f, 0.0f); // Dodger Blue
	checkError("initRendering");
}

void init(void)
{
	// Perspective projection matrix.
	projection = glm::perspective(45.0f, 800.0f / 600.0f, 1.0f, 1000.0f);
	// Load identity matrix into model matrix (no initial translation or rotation)
	// camera positioned at 20 on the z axis, looking into the screen down the -Z axis.
	//view = glm::lookAt(glm::vec3(0.0f, 0.0f, 20.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	initShader();
	initRendering();
}

/* This prints in the console when you start the program*/
void dumpInfo(void)
{
	printf("Vendor: %s\n", glGetString(GL_VENDOR));
	printf("Renderer: %s\n", glGetString(GL_RENDERER));
	printf("Version: %s\n", glGetString(GL_VERSION));
	printf("GLSL: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
	checkError("dumpInfo");
}

void ModelControls(void)
{
	// Scale kinematics
	carLength = (1.45 + 1.55) * carScale; // front wheel center offset + back wheels center offset

	float t = glutGet(GLUT_ELAPSED_TIME);		// -> Get new time
	float dt = (t - (float)old_t) / 1000.0f;	// -> Get change in time
	old_t = t;									// -> Set new time to old time

	updateSpeed(speed, dt);
	updateAngle(dt);

	turnRadius = carLength / tanf(toRad(curRotAngle));

	modelRotAngle += dt * (1 / motionScaler) * 90 * turnDir * sqrt(abs(speed / maxSpeed));
	posX -= dt * (motionScaler)*speed * sin(toRad(modelRotAngle));
	posZ -= dt * (motionScaler)*speed * cos(toRad(modelRotAngle));

	float r = ((float)rand() / (RAND_MAX)) + 1;	// Max/min padding
	if (speed > maxSpeed)						// Set max speed
		speed = maxSpeed - r;
	if (speed < -0.5 * maxSpeed)				// Set min speed
		speed = -0.5 * (maxSpeed - r);

	string s = (isBreaking ? "[Breaking]" : speedDir > 0 ? "[Gas]" : speedDir < 0 ? "[Reverse]" : "");
	//cout << speed << "mph \t" << s << endl;
	cout << turnRadius << endl;
}

void FlycamControls(void)
{
	horCam += camAngSpeedModifier * camHorDir;
	verCam += camAngSpeedModifier * camVerDir;
	camX -= camSpeedModifier * camSpeedDir * sin(toRad(horCam));
	camY += camSpeedModifier * camSpeedDir * sin(toRad(verCam));
	camZ += camSpeedModifier * camSpeedDir * cos(toRad(horCam));
}

void SetModelMatrix(void)
{
	// Calculate Model Position
	modelPosition = glm::vec3(posX, -4.275, posZ);
	model = glm::translate(modelPosition) * glm::rotate(modelRotAngle, 0.0f, 1.0f, 0.0f);
}

void SetViewMatrix(void)
{
	// Calculate Camera Position
	float smoothCamTurn = 1;//turnDir* (speed / maxSpeed);
	cameraPosition = modelPosition + glm::vec3(
		_3pDistance * sinf(toRad(modelRotAngle)),		// x camera position
		_3pDistance * 0.25f,						// y camera position
		_3pDistance * cosf(toRad(modelRotAngle)));	// z camera position

	if (flyCamMode) /// Fly camera
	{
		camPosition = glm::translate(camX, camY, camZ);
		view = glm::rotate((float)verCam, 1.0f, 0.0f, 0.0f);	// Vertical camera movement - PITCH
		view *= glm::rotate((float)horCam, 0.0f, 1.0f, 0.0f);	// Horizontal camera movement - YAW
		view *= camPosition;									// Camera position
	}
	else /// Follow camera
	{
		view = glm::lookAt(cameraPosition, modelPosition, glm::vec3(0, 1, 0));
	}
}


/*This gets called when the OpenGL is asked to display. This is where all the main rendering calls go*/
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	/*
	// fly camera
	limbscale = glm::scale(0.25f, 1.25f, 0.25f); // FIXED Values
	offset = glm::translate(0.0f, -1.0f, 0.0f);  // FIXED Values
	jointA = triangleWave(steps); // joint angle movement
	rotationRight = glm::rotate(-jointA, 1.0f, 0.0f, 0.0f);
	rotationLeft = glm::rotate(jointA, 1.0f, 0.0f, 0.0f);

	position = glm::translate(posX, -1.5f, posZ);
	model = position * glm::rotate(curRotAngle, 0.0f, 1.0f, 0.0f);
	*/
	ModelControls();		// Set model motion values
	if (flyCamMode)
		FlycamControls();	// Set fly camera motion values

	SetModelMatrix();	// Set Model
	SetViewMatrix();	// Set View
	/*
	torso->render(view * model * glm::scale(0.8f, 1.0f, 0.8f), projection); // Render current active model.
	// head is a child of the torso (only render in flycamera mode)
	if(!fpMode)
		head->render(view * model * glm::translate(0.0f, 1.5f, 0.0f) * glm::scale(0.5f, 0.5f, 0.5f), projection);
	// viewmodelmatrix * translate * rotate * translate * rescaling
	// arms are a child of the torso
	limb->render(view * model * glm::translate(1.0f, 0.75f, 0.0f) * rotationRight * offset * limbscale, projection);
	limb->render(view * model * glm::translate(-1.0f, 0.75f, 0.0f) * rotationLeft * offset * limbscale, projection);
	// legs are a child of the torso
	limb->render(view * model * glm::translate(0.4f, -1.25f, 0.0f) * rotationRight * offset * limbscale, projection);
	limb->render(view * model * glm::translate(-0.4f, -1.25f, 0.0f) * rotationLeft * offset * limbscale, projection);
	*/

	/* Car Rendering */
	car->render(view * model * glm::scale(1.0f, 1.0f, 1.0f), projection);	// Car
	float tireScale = 0.0075 * carScale;
	wheel->render(view * model * glm::translate(1.0f, -0.75f, -1.6f) * glm::rotate(curRotAngle, 0.0f, 1.0f, 0.0f) * glm::scale(tireScale, tireScale, tireScale), projection);
	wheel->render(view * model * glm::translate(-1.0f, -0.75f, -1.6f) * glm::rotate(curRotAngle, 0.0f, 1.0f, 0.0f) * glm::scale(tireScale, tireScale, tireScale), projection);

	/* Scenery, props, and terrain rendering */
	float start, sep = 10.0f, len = 160.0f, wid = 40.0f, margin = 10.0f;
	int tot = (int)((len - 2.0f * margin) / sep);	// Number of wall pieces
	start = 0.5f * (margin - len);					// Start point
	plane->render(view * glm::translate(0.0f, -5.0f, 0.0f) * glm::scale(wid, 1.0f, len), projection);	// Plain
	for (int i = 0; i < tot; i++)	// Wall pieces
	{
		box->render(view * glm::translate(wid - 0.5f * margin, -4.0f, 2.0f * (start + i * sep)) * glm::scale(3.0f, 4.0f, 3.0f), projection);
		box->render(view * glm::translate(0.5f * margin - wid, -4.0f, 2.0f * (start + i * sep)) * glm::scale(3.0f, 4.0f, 3.0f), projection);
	}

	/*
	//CUSTOM scenery, props, and terrain loading
	box->render(view * glm::translate(-15.0f, -4.0f, 0.0f) * glm::scale(2.0f, 2.0f, 30.0f), projection);	// Main wall
	ball->render(view * glm::translate(-6.0f, -4.0f, -8.0f) * glm::scale(1.0f, 1.0f, 1.0f), projection);	// Ball
	tree->render(view * glm::translate(6.0f, -5.0f, -8.0f) * glm::scale(0.1f, 0.1f, 0.1f), projection);	// TREE
	coilthing->render(view * glm::translate(-15.0f, -2.0f, 0.0f) * glm::scale(0.05f, 0.05f, 0.05f), projection); // COILTHING
	trafficcone->render(view * glm::translate(-10.0f, -5.0f, 5.0f) * glm::scale(0.02f, 0.02f, 0.02f), projection);	// TRAFFICCONE
	wheel->render(view * glm::translate(15.0f, -5.0f, 15.0f) * glm::rotate(-45.0f, 0.0f, 1.0f, 0.0f) * glm::scale(0.02f, 0.02f, 0.02f), projection); // SPAREWHEEL
	flashlight->render(view * glm::translate(-15.0f, -1.75f, 7.5f) * glm::rotate(-90.0f, 1.0f, 0.0f, 0.0f) * glm::scale(0.02f, 0.02f, 0.02f), projection); // COILTHING
	*/
	glutSwapBuffers(); // Swap the buffers.
	checkError("display");
}

/*This gets called when nothing is happening (OFTEN)*/
void idle(void)
{
	glutPostRedisplay(); // create a display event. Display calls as fast as CPU will allow when put in the idle function
}

/*Called when the window is resized*/
void reshape(int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	checkError("reshape");
}

/*Called when a normal key is pressed*/
/*
void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 27: // this is an ascii value
		exit(0);
		break;
		//case 'c': // toggle camera mode
		//	// Reset fly camera position directly behind character when exiting fpMode
		//	camX = -posX - flyCamResetOffset * sin(toRad(curRotAngle));
		//	camZ = -posZ - flyCamResetOffset * cos(toRad(curRotAngle));
		//	horCam = -curRotAngle;
		//	fpMode = !fpMode;
		//	break;
		//
	case 'r':	// reset character position
		resetChar();
		break;

		//case 'h':	// reset flycamera position
		//	fpMode = false;
		//	resetCam();
		//	break;

	case 'a':	// turn left
		curRotAngle += turn;
		//steps = 0;
		break;
	case 'd':	// turn right
		curRotAngle -= turn;
		//steps = 0;
		break;

	case 'w':	// turn left
		//steps--;
		posX -= speed * sin(toRad(curRotAngle));
		posZ -= speed * cos(toRad(curRotAngle));
		break;
	case 's':	// turn right
		//steps++;
		posX += speed * sin(toRad(curRotAngle));
		posZ += speed * cos(toRad(curRotAngle));
		break;

	case 'f':	// camera move forward
		camX -= sin(toRad(horCam));
		camY += sin(toRad(verCam));
		camZ += cos(toRad(horCam));
		break;
	case 'v':	// camera move backward
		camX += sin(toRad(horCam));
		camY -= sin(toRad(verCam));
		camZ -= cos(toRad(horCam));
		break;
		}
		printf("Key: %c\n", key);
		printf("ForX: %f\n", camX);
		printf("ForZ: %f\n", camZ);
		printf("Count = %d\n", (int)steps);
	}
}
*/

void OnToggleFlycam(void)
{
	camX = -posX - flyCamResetOffset * sin(toRad(curRotAngle));
	camZ = -posZ - flyCamResetOffset * cos(toRad(curRotAngle));
	horCam = -curRotAngle;
}

void KeyDown(unsigned char key, int x, int y)	// Keydown events
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
		OnToggleFlycam();
		break;
	}
}
void KeyUp(unsigned char key, int x, int y)		// Keydown events
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

		/// FLYCAMERA CONTROLS ///
	case 'f':	// camera move forward
		camSpeedDir = 0;
		break;
	case 'v':	// camera move backward
		camSpeedDir = 0;
		break;
	}
}

void SpecialInputDown(int key, int x, int y)
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
void SpecialInputUp(int key, int x, int y)
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

/*
void specialInput(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_UP:
		if (verCam > -45)
			verCam-=5;
		break;
	case GLUT_KEY_DOWN:
		if (verCam < 45)
			verCam+=5;
		break;
	case GLUT_KEY_LEFT:  // look left
		horCam-=5;
		break;
	case GLUT_KEY_RIGHT: // look right
		horCam+=5;
		break;
	}
	printf("Hor: %d\n", horCam);
	printf("Ver: %d\n", verCam);
	glutPostRedisplay();
}
*/
int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(800, 600);
	glutInitWindowPosition(100, 100);
	glutCreateWindow(argv[0]);
	glewInit();
	dumpInfo();
	init();
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(KeyDown);
	glutKeyboardUpFunc(KeyUp);
	glutSpecialFunc(SpecialInputDown);
	glutSpecialUpFunc(SpecialInputUp);
	glEnable(GL_DEPTH_TEST);

	// Provided props
	plane = new Model(&shader, "models/plane.obj");
	car = new Model(&shader, "models/dodge-challenger_model.obj", "models/");
	box = new Model(&shader, "models/unitcube.obj", "models/");
	wheel = new Model(&shader, "models/wheel.obj", "models/");

	//ball = new Model(&shader, "models/sphere.obj");

	// Custom props
	/*
	tree = new Model(&shader, "models/tree.obj", "models/");
	coilthing = new Model(&shader, "models/coilthing.obj", "models/");
	trafficcone = new Model(&shader, "models/trafficcone.obj", "models/");
	flashlight = new Model(&shader, "models/flashlight.obj", "models/");
	*/
	/*
	torso = new Model(&shader, "models/cylinder.obj"); // Torso
	//sphere = new Model(&shader, "models/dodge-challenger_model.obj", "models/"); // you must specify the material path for this to load
	head = new Model(&shader, "models/cylinder.obj", "models/"); // you must specify the material path for this to load
	limb = new Model(&shader, "models/cylinder.obj", "models/"); // you must specify the material path for this to load
	*/


	glutMainLoop();

	return 0;
}