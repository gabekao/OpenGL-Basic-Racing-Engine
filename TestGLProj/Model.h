#include <GL/glew.h>
#include <glm/glm.hpp>

#include <vector>
#include <map>
#include <array>
#include "tiny_obj_loader.h"
#include <string>
#include "Shader.h"

/*This class represents a mesh - a collection of triangles. It stores a list of vertices, which are interpreted by openGL as every 3 vertices makes a triangle.*/
class Model{

public:

	/*This struct represents a vertex in 3D space*/
	struct Vertex 
	{
		Vertex() : Position(0.0f,0.0f,0.0f), Normal(0.0f,0.0f,1.0f){} // default values
		glm::vec3 Position; /// x,y,z 
		glm::vec3 Normal; // determines which way the vertex is 'facing'

	};

	struct Properties
	{
		glm::vec3 position1;
		glm::vec3 position2;
		glm::vec3 position3;
		glm::vec3 position4;
		glm::vec3 positions[16];
		glm::vec3 points[16];
	} properties;

	GLfloat varray[24]; // vertex array containing the vertices of the bounding box
	float boxVerts[6];
	bool isPlayer = false;

	/*
	@param shader - a pointer to the shader program to use
	@param filename - the name of the file with the relative directory included
	@param materialPath - [optional] the relative path to the mtl files. NOTE: if your obj has a mtl file associated with it, this is not optional!
	*/
	Model(Shader *shader, Shader* shaderBB, const char* filename, const char* materialPath = NULL, bool player = false);
	~Model(void){} // default destructor
	void render(glm::mat4 ModelView, glm::mat4 Projection, bool useMaterial); // render the model
	void renderBB(glm::mat4 ModelView, glm::mat4 Projection); // render the bounding box model
	void setProperties(glm::mat4 model);

private:
	Shader *m_shader; // shader program
	Shader* m_shaderBB; // shader program for bounding box
	std::vector<tinyobj::shape_t> shapes; //a list of meshes and their respective materials
	std::vector<GLuint> m_VBO;// vertex buffer IDs, each corresponding to a shape
	std::vector<GLuint> m_NBO;// normal buffer IDs, each corresponding to a shape
	std::vector<GLuint> m_IBO;// index buffer IDs, each corresponding to a shape
	std::vector<GLuint> m_VBOBB; // vertex buffer IDs for bounding box
	std::vector<GLuint> m_IBOBB; // index buffer IDs for bounding box

	void updateBuffers(); //initialize your VBO and update when triangles are added
	void findBounds(); // finds the AABB boundaries (min/max)
	void setVertices(float values[6]); // Sets vertices for the AABB
	void setPoints();
	void updateBuffersBB(); // initializes VBO and IBO for bounding box
};

