#include "Car.h"

class Camera
{
private:

	glm::vec3 cameraPosition;			// camera pos
	glm::mat4 camPosition;				// flycamera position matrix

	// Flycam values
	float verCam = 0;					// flycamera vertical angle
	float horCam = 0;					// flycamera horizontal angle
	float camX = 0;						// flycamera x position
	float camY = 0;						// flycamera y position
	float camZ = -20;					// flycamera z position
	float flyCamResetOffset = 7.5;		// flycam magnititudal offset distance behind player after mode toggled
	float camSpeedModifier = 0.25f;
	float camAngSpeedModifier = 1.0f;

	float camSpeedDir = 0.0f;			// Ranges from -1.0 to 1.0
	float camVerDir = 0.0f;				// Ranges from -1.0 to 1.0
	float camHorDir = 0.0f;				// Ranges from -1.0 to 1.0


	void resetFlycam(void);
	void OnToggleFlycam(Car car);

public:

	bool flyCamMode = false;			// [first person / flycamera] toggle
	glm::mat4 SetViewMatrix(Car car);
	void FlycamControls(Car car);

	void CameraKeyDown(unsigned char key, Car car);
	void CameraKeyUp(unsigned char key);
	void CameraSpecialInputDown(int key);
	void CameraSpecialInputUp(int key);

	
};

