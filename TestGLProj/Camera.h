#include <GL/freeglut_std.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform2.hpp>
#pragma once
class Camera
{
	public:

		glm::vec3 eye;
		glm::vec3 center;
		glm::vec3 forward;
		glm::vec3 up;

		Camera(glm::vec3 e, glm::vec3 c, glm::vec3 u);


		glm::vec3 getEye();

		glm::vec3 getCenter();

		glm::vec3 getUp();


		void setLookAt(glm::mat4 *view);

		void changeTarget(glm::vec3 target);
		
		void setPosition(glm::vec3 pos);

		void specialControls(int key, int x, int y);

		void normalControls(int key, int x, int y);


};

