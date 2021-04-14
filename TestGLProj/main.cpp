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
#include "Camera.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

Shader shader; // loads our vertex and fragment shaders
Model *playerCar; //a playerCar 
Model *plane; //a plane
Model *track; //a track
Camera *camera;
glm::mat4 projection; // projection matrix
glm::mat4 view; // where the camera is looking
glm::mat4 model; // where the model (i.e., the myModel) is located wrt the camera



/* report GL errors, if any, to stderr */
void checkError(const char *functionName)
{
	GLenum error;
	while (( error = glGetError() ) != GL_NO_ERROR) {
		std::cerr << "GL error " << error << " detected in " << functionName << std::endl;
	}
}

void initShader(void)
{
	shader.InitializeFromFile("shaders/phong.vert", "shaders/phong.frag");
	shader.AddAttribute("vertexPosition");
	shader.AddAttribute("vertexNormal");

	checkError ("initShader");
}

void initRendering(void)
{
	glClearColor (0.117f, 0.565f, 1.0f, 0.0f); // Dodger Blue
	checkError ("initRendering");
}

void init(void) 
{	
	// Perspective projection matrix.
	projection = glm::perspective(45.0f, 800.0f/600.0f, 1.0f, 1000.0f);
	camera = new Camera(glm::vec3(0.f, 0.f, -30.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
	// Load identity matrix into model matrix (no initial translation or rotation)
	

	initShader ();
	initRendering ();
}

/* This prints in the console when you start the program*/
void dumpInfo(void)
{
	printf ("Vendor: %s\n", glGetString (GL_VENDOR));
	printf ("Renderer: %s\n", glGetString (GL_RENDERER));
	printf ("Version: %s\n", glGetString (GL_VERSION));
	printf ("GLSL: %s\n", glGetString (GL_SHADING_LANGUAGE_VERSION));
	checkError ("dumpInfo");
}

bool initial = false;
float angle = -90.f;
/*This gets called when the OpenGL is asked to display. This is where all the main rendering calls go*/
void display(void)
{
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	// Adding rotation to model matrix
	//model = glm::rotate(model, glm::radians(10.f), glm::vec3(0.f, 1.f, 0.f));

	glm::mat4 carTrans;
	carTrans = model * glm::translate(0.f, 0.f, 0.f) * glm::rotate(angle, glm::vec3(0.f, 1.f, 0.f));
	//camera->changeTarget(glm::vec3(carTrans[3]) + camera->forward);
	camera->setPosition(glm::vec3(carTrans[3]) + camera->position);

	//camera->changeTarget(glm::vec3(carTrans[3]));
	camera->setLookAt(&view);

	//carTrans = glm::translate(carTrans, )

	
	playerCar->render(glm::mat4(1.f) * glm::translate(0.f, -2.f, -7.f), projection); // Render current active model.

	// track is a child of the playerCar
	//track->render(view * glm::translate(0.0f, -5.0f,0.0f), projection);

	plane->render(view * glm::translate(0.0f, -1.f,0.0f)*glm::scale(20.0f,1.0f,20.0f), projection);
	
	glutSwapBuffers(); // Swap the buffers.
	checkError ("display");
}

/*This gets called when nothing is happening (OFTEN)*/
void idle()
{
	glutPostRedisplay(); // create a display event. Display calls as fast as CPU will allow when put in the idle function
}

/*Called when the window is resized*/
void reshape (int w, int h)
{
	glViewport (0, 0, (GLsizei) w, (GLsizei) h);
	checkError ("reshape");
}

/*Called when a normal key is pressed*/
void keyboard(unsigned char key, int x, int y)
{
	camera->normalControls(key, x, y);
	std::cout << camera->yaw << '\n';

	switch (key) {
	case 'w':
		model = glm::translate(model, glm::vec3(0.f, 0.f, 1.f));
		//view *= glm::rotate(2.f, glm::vec3(0.f, 1.f, 0.f));
		std::cout << glm::to_string(view) << '\n';
		break;
	case 's':
		model = glm::translate(model, glm::vec3(0.f, 0.f, -1.f));
		break;
	case 'a':
		model *= glm::rotate(2.f, glm::vec3(0.f, 1.f, 0.f));
		break;
	case 'd':
		model *= glm::rotate(-2.f, glm::vec3(0.f, 1.f, 0.f));
		break;
	case 27: // this is an ascii value
		exit(0);
		break;	
	}
}

void specialKeyboard(int key, int x, int y)
{
	camera->specialControls(key, x, y);
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode (GLUT_DOUBLE| GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize (800, 600); 
	glutInitWindowPosition (100, 100);
	glutCreateWindow (argv[0]);
	glewInit();
	dumpInfo ();
	init ();
	glutDisplayFunc(display); 
	glutIdleFunc(idle); 
	glutReshapeFunc(reshape);
	glutKeyboardFunc (keyboard);
	glutSpecialFunc(specialKeyboard);
	glEnable(GL_DEPTH_TEST);


	playerCar = new Model(&shader, "models/player_car.obj", "models/");
	plane = new Model(&shader, "models/plane.obj");
	track = new Model(&shader, "models/racetrack1.obj", "models/"); // you must specify the material path for this to load
	

	glutMainLoop();

	return 0;
}