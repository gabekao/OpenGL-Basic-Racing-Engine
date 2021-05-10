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

#define SIZE 24

using namespace std;

Shader shader;		// loads our model vertex and fragment shaders
Shader shaderBB;	// loads our boundary box vertex and fragment shaders
Shader shaderText;

Car car;
Camera camera;
Model* player;
Model* box;
Model* plane;
Model* wheel;
Model* light;
Model* trunk;
Model* tree;
Model* trunks[SIZE];

Text* text;

struct GameObjects {
	Model* obj;
	glm::mat4 model;
} gameobjects[SIZE];

struct Spotlight {
	glm::vec4 pos, dir;
} headlights[2];

glm::mat4 projection;		// projection matrix
glm::mat4 view;				// where the camera is lookin
glm::mat4 model;			// where the model (i.e., the myModel) is located wrt the camera
glm::vec4 lightPosition;	// Light position
glm::vec4 spotlightPosisition;
glm::vec4 spotlightDirection;

float FRAME_TIME = 16.66667;
float previousTime = 0;
bool displayBB = false;		// Display bounding box
bool useCTM = false;		// Use Cook-Torrence Model
bool stop = false;
bool created = false;

bool CheckCollision();
void UseLight();
void DrawTerrain();
void CreateTerrain();


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
	glClearColor(0.0f, 0.8f, 0.8f, 0.0f);
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
	spotlightPosisition = glm::vec4(0.0f, 10.0f, 0.0f, 1.0f);
	spotlightDirection = glm::vec4(0.0f, -1.0f, 0.0f, 0.0f);

	headlights[0].pos = glm::vec4(-0.6f, 0.0f, -2.5f, 1.0f);
	headlights[0].dir = glm::vec4(-0.1f, -0.18f, -1.0f, 0.0f);
	headlights[1].pos = glm::vec4(0.6f, 0.0f, -2.5f, 1.0f);
	headlights[1].dir = glm::vec4(0.1f, -0.18f, -1.0f, 0.0f);

	initShader();
	initRendering();

	// Initializes text class based on provided font
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

		// Drawing trees
		DrawTerrain();

		// Setting collision points for player
		player->setProperties(model);

		// Check if car collided
		
		if (CheckCollision())
			car.carCollided = true;
		else
			car.carCollided = false;
			
		
		// Use point light
		UseLight();


		// Rendering race track
		plane->render(view * glm::translate(0.0f, -5.0f, 0.0f) * glm::scale(2.0f, 2.0f, 2.0f), projection, true);

		/* Car Rendering */
		player->render(view * model * glm::scale(1.0f, 1.0f, 1.0f), projection, true);	// Car
		wheel->render(view * model * glm::translate(1.0f, -0.75f, -1.6f) * glm::rotate(car.curRotAngle, 0.0f, 1.0f, 0.0f) * glm::scale(1.0f, 1.0f, 1.0f), projection, true);
		wheel->render(view * model * glm::translate(-1.0f, -0.75f, -1.6f) * glm::rotate(car.curRotAngle + 180, 0.0f, 1.0f, 0.0f) * glm::scale(1.0f, 1.0f, 1.0f), projection, true);

		// Headlights position
		if (displayBB) {
			player->renderBB(view * model, projection);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			box->render(view * model * glm::translate(-0.6f, 0.1f, -2.7f) * glm::scale(.2f, .2f, .2f), projection, true);
			box->render(view * model * glm::translate(0.6f, 0.1f, -2.7f) * glm::scale(.2f, .2f, .2f), projection, true);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		// String to hold speed
		std::string spd;

		// Convert car speed float to string
		spd = std::to_string(car.speed);

		// Speed set to two point precision
		spd = "Speed: " + spd.substr(0, spd.find(".") + 3);

		// Render speed
		text->RenderText(spd, 10.0, 560.0, 0.5, glm::vec3(1.0, 1.0, 1.0));

		// String to hold timer
		std::string timer, sec, ms;

		// Format time by seconds
		if (currentTime > 10.0)
			timer = to_string(currentTime / 1000.0).substr(0, 4);
		else if (currentTime > 100.0)
			timer = to_string(currentTime / 1000.0).substr(0, 5);
		else
			timer = to_string(currentTime / 1000.0).substr(0, 2);

		// Convert timer float to string
		timer = "Timer: " + timer;

		// Render timer
		text->RenderText(timer, 360.0, 560.0, 0.5, glm::vec3(1.0, 1.0, 1.0));


		
		glutSwapBuffers(); // Swap the buffers.
		checkError("display");
		
	}
	previousTime = currentTime;
}

/*Checks if car collids with one box*/
bool CheckCollision()
{
	// Initialize collisions
	bool collisionX = false, collisionY = false, collisionZ = false;
	
	for (int i = 0; i < SIZE; i++)
	{
		// Check collision based on points on edge of car
		for (int j = 0; j < 16; j++)
		{
			// Check collision on x
			collisionX = (player->properties.positions[j].x <= gameobjects[i].obj->properties.position2.x &&
				player->properties.positions[j].x >= gameobjects[i].obj->properties.position1.x);

			// Check collision on y
			collisionY = (player->properties.positions[j].y <= gameobjects[i].obj->properties.position2.y &&
				player->properties.positions[j].y >= gameobjects[i].obj->properties.position1.y);

			// Check collision on z
			collisionZ = (player->properties.positions[j].z <= gameobjects[i].obj->properties.position2.z &&
				player->properties.positions[j].z >= gameobjects[i].obj->properties.position1.z);

			// Return if both true
			if (collisionX && collisionZ)
				return true;
		}
	}
	return false;
}

float angle = 0;
bool spot = false;
bool lightOn = false;

/*Enables lights to be rendered*/
void UseLight()
{
	angle += 0.002f;

	glm::vec4 lightPos, spotlightPos;
	lightPos = glm::rotate(angle, 0.0f, 0.0f, -1.0f) * lightPosition;
	spotlightPos = spotlightPosisition;

	shader.Activate();
	shader.SetUniform("useCTM", useCTM); // Toggle Cook-Torrance Model
	shader.SetUniform("lightPosition", view * lightPos);
	shader.SetUniform("lightDiffuse", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	shader.SetUniform("lightSpecular", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	shader.SetUniform("lightAmbient", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

	if (spot)
	{
		// Set stationary spotlight uniforms (pos, dir, angle, exp)
		int aLen = sizeof(headlights) / sizeof(Spotlight);
		for (int i = 0; i < aLen; i++) {
			shader.SetUniform("headlightIndex", i);

			shader.SetUniform("headlights[" + to_string(i) + "].position", model * headlights[i].pos);
			shader.SetUniform("headlights[" + to_string(i) + "].direction", glm::normalize(model * headlights[i].dir));
		}
		shader.SetUniform("cutOffAngle", 15.0f);
		shader.SetUniform("spotlightExponent", 60.0f);

		// Determines state in fragment shader
		shader.SetUniform("spotlightActive", spot);
		shader.SetUniform("flashOn", false);
		shader.SetUniform("View", view);
	}
	else
	{
		// Check if light is on/off
		if (lightOn)
		{
			// Set light uniforms (pos, dir, angle, exp)
			shader.SetUniform("spotlightPosition", glm::vec4(0.f));
			shader.SetUniform("spotlightDirection", glm::vec4(0.f, 0.f, -1.f, 0.f));
			shader.SetUniform("cutOffAngle", 8.0f);
			shader.SetUniform("spotlightExponent", 150.0f);

			// Determines state in fragment shader (sets light on)
			shader.SetUniform("spotlightActive", spot);
			shader.SetUniform("flashOn", lightOn);
		}
		else
		{
			// Determines state in fragment shader (sets light off)
			shader.SetUniform("spotlightActive", spot);
			shader.SetUniform("flashOn", lightOn);
		}
	}

	// Rendering light object
	light->render(view * glm::translate(lightPos.x, lightPos.y, lightPos.z), projection, false);

	shader.DeActivate();
}

/*Initializes tree game objects split into subsections*/
void CreateTerrain()
{
	// Iterator
	int i = 0;

	// Min/max for x and z
	int minX, maxX, minZ, maxZ;
	
	// Loop for each subsection (3)
	for (int n = 0; n < 3; n++)
	{
		// Depending on subsection, set appropriate min/max
		if (n == 0)
		{
			minX = -30;
			maxX = 52;
			minZ = -44;
			maxZ = 71;
		}
		else if (n == 1)
		{
			minX = -70;
			maxX = 40;
			minZ = -44;
			maxZ = 36;
		}
		else
		{
			minX = 26;
			maxX = 44;
			minZ = -64;
			maxZ = 54;
		}

		// For each subsection create a tree game object
		for (i = i; i < SIZE/3*(n+1); i++)
		{
			// Find a random x and z in range
			float x = rand() % (maxX + 1) + minX;
			float z = rand() % (maxZ + 1) + minZ;

			// Create a trunk for collision purposes
			trunks[i] = new Model(&shader, &shaderBB, "models/trunk.obj", "models/");
			
			// Add model to game object
			gameobjects[i].obj = trunks[i];

			// Add model matrix to game object
			gameobjects[i].model = glm::translate(x, -5.0f, z);

			// Set collider properties for game object
			gameobjects[i].obj->setProperties(gameobjects[i].model);
		}
	}

}


/*Renders tree game objects on the scene*/
void DrawTerrain()
{
	// Check to see if terrain has been created
	if (!created)
	{
		// Create terrain (initiliaze game objects)
		CreateTerrain();
		// Set created to true
		created = true;
	}

	// Render each game object
	for (int i = 0; i < SIZE; i++)
	{
		gameobjects[i].obj->render(view * gameobjects[i].model, projection, true);
		tree->render(view * gameobjects[i].model, projection, true);
	}
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
	case 'f':
		lightOn = !lightOn;
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
	player = new Model(&shader, &shaderBB, "models/Dodge_no_wheels.obj", "models/", true);
	box = new Model(&shader, &shaderBB, "models/cube.obj", "models/");
	wheel = new Model(&shader, &shaderBB, "models/wheel.obj", "models/");
	light = new Model(&shader, &shaderBB, "models/old/sphere.obj", "models/old/");
	trunk = new Model(&shader, &shaderBB, "models/trunk.obj", "models/");
	tree = new Model(&shader, &shaderBB, "models/treeleaf.obj", "models/");


	glutMainLoop();

	return 0;
}