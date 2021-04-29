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

using namespace std;

Shader shader; // loads our vertex and fragment shaders
Shader shaderBB; // loads our vertex and fragment shaders

Car car;
Camera camera;
Model* player;
Model* box;
Model* plane;
Model* wheel;

glm::mat4 projection;		// projection matrix
glm::mat4 view;				// where the camera is lookin
glm::mat4 model;			// where the model (i.e., the myModel) is located wrt the camera

float FRAME_TIME = 16.66667;
float previousTime = 0;
bool displayBB = false;

bool CheckCollision(glm::mat4 objP, glm::mat4 obj);

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


int frameCounter = 0;
/*This gets called when the OpenGL is asked to display. This is where all the main rendering calls go*/
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	float currentTime = glutGet(GLUT_ELAPSED_TIME);
	if (previousTime + FRAME_TIME > currentTime)
	{
		car.CarControls();		// Set model motion values
		if (camera.flyCamMode)
			camera.FlycamControls(car);	// Set fly camera motion values
		
		float angle;
		glm::mat4 m;
		model = car.SetCarModelMatrix(&m, &angle);	// Set Model
		view = camera.SetViewMatrix(car);	// Set View


		/* Car Rendering */

		//player->render(view * model * glm::scale(1.0f, 1.0f, 1.0f), projection, true);	// Car
		float tireScale = 0.0075f;

		wheel->render(view * model * glm::translate(1.0f, -0.75f, -1.6f) * glm::rotate(car.curRotAngle, 0.0f, 1.0f, 0.0f) * glm::scale(tireScale, tireScale, tireScale), projection, false);
		wheel->render(view * model * glm::translate(-1.0f, -0.75f, -1.6f) * glm::rotate(car.curRotAngle, 0.0f, 1.0f, 0.0f) * glm::scale(tireScale, tireScale, tireScale), projection, false);

		/* Scenery, props, and terrain rendering */
		float start, sep = 10.0f, len = 160.0f, wid = 40.0f, margin = 10.0f;
		int tot = (int)((len - 2.0f * margin) / sep);	// Number of wall pieces
		start = 0.5f * (margin - len);					// Start point
		plane->render(view * glm::translate(0.0f, -5.0f, 0.0f) * glm::scale(2.f, 2.f, 2.f), projection, true);	// Plain
		/*
		for (int i = 0; i < tot; i++)	// Wall pieces
		{
			box->render(view * glm::translate(wid - 0.5f * margin, -4.0f, 2.0f * (start + i * sep)) * glm::scale(3.0f, 4.0f, 3.0f), projection,false);
			box->render(view * glm::translate(0.5f * margin - wid, -4.0f, 2.0f * (start + i * sep)) * glm::scale(3.0f, 4.0f, 3.0f), projection,false);
		}*/

		//box->render(view * glm::translate(0.f, -4.275f, -5.f), projection, false);

		glm::mat4 obj = glm::translate(-1.f, -4.275f, -5.f);

		box->setProperties(obj);
		player->setProperties(model* glm::translate(-player->properties.size.x / 2, 0.f, -player->properties.size.z / 2));
		glm::mat4 a = model * glm::translate(-player->properties.size.x / 2, 0.f, -player->properties.size.z / 2);
		if (CheckCollision(model, obj))
		{
			if ((frameCounter % 60) == 0)
			{
				printf("Collided\n");
				//std::cout << glm::to_string(model) << std::endl;
			}
		}
		if (!displayBB)
		{
			if ((frameCounter % 60) == 0)
			{
				//printf("Collided\n");
				std::cout << glm::to_string(a) << std::endl;
			}
			box->renderBB(view * obj, projection);
			player->renderBB(view * model, projection);
			//box->renderBB(view *glm::translate(player->properties.position) *
				//glm::translate(player->properties.center) * glm::rotate(angle, 0.f, 1.f, 0.f) * glm::translate(-player->properties.center), projection);
			
			box->renderBB(view * model * glm::translate(-player->properties.size.x/2, 0.f, -player->properties.size.z/2), projection);

			/*
			printf("Car  -- x: %f, z: %f\n", car.posX, car.posZ);
			printf("Size -- x: %f, z: %f\n", player->properties.size.x, player->properties.size.z);
			printf("Box  -- x: %f, z: %f\n", obj[3][0], obj[3][2]);
			printf("Size -- x: %f, z: %f\n", box->properties.size.x, box->properties.size.z);
			*/
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
		frameCounter++;
	}
	previousTime = currentTime;
}

bool CheckCollision(glm::mat4 objP, glm::mat4 obj)
{

	//bool collisionX = player->properties.center.x + player->properties.size.x >= box->properties.position.x && 
		//box->properties.position.x + box->properties.size.x >= player->properties.position.x;

	bool collisionX = objP[3][0] >= box->properties.position.x &&
		box->properties.position.x + box->properties.size.x >= objP[3][0];

	//bool collisionZ = player->properties.position.z + player->properties.size.z >= box->properties.position.z &&
		//box->properties.position.z + box->properties.size.z >= player->properties.position.z;

	bool collisionZ = objP[3][2] >= box->properties.position.z &&
		box->properties.position.z + box->properties.size.z >= objP[3][2];

	return collisionX && collisionZ;
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



void KeyDown(unsigned char key, int x, int y)	// Keydown events
{
	car.CarKeyDown(key);

	camera.CameraKeyDown(key, car);

	switch (key){
	case 'p':
		displayBB = !displayBB;
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
	player = new Model(&shader, &shaderBB, "models/car.obj", "models/");
	box = new Model(&shader, &shaderBB, "models/old/unitcube.obj", "models/old/");
	wheel = new Model(&shader, &shaderBB, "models/wheel.obj", "models/");

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