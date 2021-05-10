#include <GL/glew.h>
#include <GL/freeglut.h>

//glm library
#include <glm/glm.hpp>
#include <glm/gtx/transform2.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include "Model.h"
#include "Shader.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <math.h>


#include "Car.h"
#include "Camera.h"
#include "Text.h"

using namespace std;

Shader shader; // loads our vertex and fragment shaders
Shader shaderBB; // loads our vertex and fragment shaders
Shader shaderText;

Car car;
Camera camera;
Model* player;
Model* box;
Model* plane;
Model* wheel;
Model* light;
Text* text;

glm::mat4 projection;		// projection matrix
glm::mat4 view;				// where the camera is lookin
glm::mat4 model;			// where the model (i.e., the myModel) is located wrt the camera
glm::vec4 lightPosition;	// Light position


float FRAME_TIME = 16.66667;
float previousTime = 0;
bool displayBB = false;		// Display bounding box
bool useCTM = false;		// Use Cook-Torrence Model
bool stop = false;

bool CheckCollision();
void UseLight();

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

	// BB shader
	shaderBB.InitializeFromFile("shaders/phong.vert", "shaders/green.frag");
	shaderBB.AddAttribute("vertexPosition");
	shaderBB.AddAttribute("vertexNormal");

	shaderText.InitializeFromFile("shaders/text.vert", "shaders/text.frag");


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
	Car car;
	Camera camera;


	lightPosition = glm::vec4(0.0f, 100.0f, 0.0f, 1.0f);



	initShader();
	initRendering();

	text = new Text(&shaderText, "fonts/Antonio-Regular.ttf");

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


int frameCounter = 0;
/*This gets called when the OpenGL is asked to display. This is where all the main rendering calls go*/
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	float currentTime = glutGet(GLUT_ELAPSED_TIME);

	if (previousTime + FRAME_TIME > currentTime)
	{
		// Set motion values
		car.CarControls();

		// Check to see if fly cam is on
		if (camera.flyCamMode)
			camera.FlycamControls(car);
		

		// Create car model matrix and view matrix
		model = car.SetCarModelMatrix();
		view = camera.SetViewMatrix(car);

		// Mat4 for collision box test
		glm::mat4 obj = glm::translate(-1.f, -4.275f, -5.f);

		// Setting collision points for box and player
		box->setProperties(obj);
		player->setProperties(model);

		// Check if car collided
		
		if (CheckCollision())
			car.carCollided = true;
		else
			car.carCollided = false;
			
		
		// Use point light
		UseLight();

		// Rendering test box
		box->render(view * obj, projection, false);

		// Rendering race track
		plane->render(view * glm::translate(0.0f, -5.0f, 0.0f) * glm::scale(2.f, 2.f, 2.f), projection, true);

		/* Car Rendering */
		player->render(view * model * glm::scale(1.0f, 1.0f, 1.0f), projection, true);	// Car
		float tireScale = 0.0075f;
		wheel->render(view * model * glm::translate(1.0f, -0.75f, -1.6f) * glm::rotate(car.curRotAngle, 0.0f, 1.0f, 0.0f) * glm::scale(tireScale, tireScale, tireScale), projection, false);
		wheel->render(view * model * glm::translate(-1.0f, -0.75f, -1.6f) * glm::rotate(car.curRotAngle, 0.0f, 1.0f, 0.0f) * glm::scale(tireScale, tireScale, tireScale), projection, false);

		if (displayBB)
			player->renderBB(view * model, projection);

		
		std::string spd;
		spd = std::to_string(car.speed);
		spd = "Speed: " + spd.substr(0, spd.find(".") + 3);
		text->RenderText(spd, 50.0, 550.0, 0.5, glm::vec3(0.0, 0.0, 0.0));

		

		/* Scenery, props, and terrain rendering */
		/*
		float start, sep = 10.0f, len = 160.0f, wid = 40.0f, margin = 10.0f;
		int tot = (int)((len - 2.0f * margin) / sep);	// Number of wall pieces
		start = 0.5f * (margin - len);					// Start point
		plane->render(view * glm::translate(0.0f, -5.0f, 0.0f) * glm::scale(2.f, 2.f, 2.f), projection, true);	// Plain
		
		for (int i = 0; i < tot; i++)	// Wall pieces
		{
			box->render(view * glm::translate(wid - 0.5f * margin, -4.0f, 2.0f * (start + i * sep)) * glm::scale(3.0f, 4.0f, 3.0f), projection,false);
			box->render(view * glm::translate(0.5f * margin - wid, -4.0f, 2.0f * (start + i * sep)) * glm::scale(3.0f, 4.0f, 3.0f), projection,false);
		}*/

		
		glutSwapBuffers(); // Swap the buffers.
		checkError("display");
		//frameCounter++;
	}
	previousTime = currentTime;
}

/*Checks if car collids with one box*/
bool CheckCollision()
{
	// Initialize collisions
	bool collisionX = false, collisionZ = false;
	
	// Check collision based on points on edge of car
	for (int i = 0; i < 16; i++)
	{
		// Check collision on x
		collisionX = (player->properties.positions[i].x <= box->properties.position2.x &&
			player->properties.positions[i].x >= box->properties.position1.x);

		// Check collision on z
		collisionZ = (player->properties.positions[i].z <= box->properties.position2.z &&
			player->properties.positions[i].z >= box->properties.position1.z);

		// Return if both true
		if (collisionX && collisionZ)
			return true;
	}

	return false;
}

float angle = 0;
bool spot = false;
void UseLight()
{
	angle += 0.002f;

	glm::vec4 lightPos, leftLight;
	lightPos = glm::rotate(angle, 0.0f, 0.0f, -1.0f) * lightPosition;


	shader.Activate();
	shader.SetUniform("useCTM", useCTM); // Toggle Cook-Torrance Model
	shader.SetUniform("lightPosition", view * lightPos);
	shader.SetUniform("lightDiffuse", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	shader.SetUniform("lightSpecular", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	shader.SetUniform("lightAmbient", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	// Rendering light object
	light->render(view * glm::translate(lightPos.x, lightPos.y, lightPos.z), projection, false);

	shader.DeActivate();
}



/*This gets called when nothing is happening (OFTEN)*/
void idle(void)
{
	if (!stop)
		glutPostRedisplay(); // create a display event. Display calls as fast as CPU will allow when put in the idle function
}

/*Called when the window is resized*/
void reshape(int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	checkError("reshape");
}



void KeyDown(unsigned char key, int x, int y)	// Keydown events
{
	car.CarKeyDown(key);

	camera.CameraKeyDown(key, car);

	switch (key){
	case 'p':
		displayBB = !displayBB;
		break;
	case 'l':
		stop = !stop;
		break;
	case 'm':
		useCTM = !useCTM;
		break;
	case 'o':
		spot = !spot;
		break;
	}
}

void KeyUp(unsigned char key, int x, int y)		// Keydown events
{
	car.CarKeyUp(key);

	camera.CameraKeyUp(key);
}

void SpecialInputDown(int key, int x, int y)
{
	camera.CameraSpecialInputDown(key);
}

void SpecialInputUp(int key, int x, int y)
{
	camera.CameraSpecialInputUp(key);
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
	plane = new Model(&shader, &shaderBB, "models/racetrackroad.obj", "models/");
	player = new Model(&shader, &shaderBB, "models/car.obj", "models/", true);
	box = new Model(&shader, &shaderBB, "models/cube.obj", "models/");
	wheel = new Model(&shader, &shaderBB, "models/wheel.obj", "models/");
	light = new Model(&shader, &shaderBB, "models/old/sphere.obj", "models/old/");

	

	glutMainLoop();

	return 0;
}