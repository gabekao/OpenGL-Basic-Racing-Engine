#include "Model.h"

GLuint indices[]{
		1,3, 3,7, 7,5, 5,1,
		0,2, 2,6, 6,4, 4,0,
		0,1, 2,3, 4,5, 6,7
};

Model::Model(Shader *shader, Shader* shaderBB, const char* filename, const char* materialPath, bool player)
{
	isPlayer = player;

	m_shader = shader;
	tinyobj::LoadObj(shapes,filename, materialPath);
	for(int i = 0; i<shapes.size(); i++){
		m_VBO.push_back(0);
		m_NBO.push_back(0);
		m_IBO.push_back(0);
		glGenBuffers(1, &m_VBO[i]); // generate a 'name' for the VBO
		glGenBuffers(1, &m_NBO[i]); // generate a 'name' for the NBO
		glGenBuffers(1, &m_IBO[i]); // generate a 'name' for the IBO
		// Bind ibo to the index buffer.
		
	}
	
	updateBuffers();
	
	// Bounding Box buffers
	findBounds(); // Finding bounding box bounds
	m_shaderBB = shaderBB; // new shader for bounding box
	for (int i = 0; i < shapes.size(); i++)
	{
		m_VBOBB.push_back(0);
		m_IBOBB.push_back(0);
		glGenBuffers(1, &m_VBOBB[i]); // generate a 'name' for the VBO
		glGenBuffers(1, &m_IBOBB[i]); // generate a 'name' for the IBO
	}

	updateBuffersBB(); // calling update buffers for bounding box
}


/*
Render the mesh to the screen
@param Modelview - the model view matrix that defines where the camera is looking
@param Projection - the projection matrix that defines how 3D vertices are projected on the 2D screen.
*/
void Model::render(glm::mat4 ModelView, glm::mat4 Projection, bool useMaterial) {

	m_shader->Activate(); // Bind shader.

	//update the variables in the shader program
	m_shader->SetUniform("Projection", Projection); // send projection to vertex shader
	m_shader->SetUniform("ModelView", ModelView);  // send modelview to vertex shader
	//m_shader->SetUniform("lightPosition", glm::vec4(1.0, 0.0, 0.0, 1.0)); // send light position to vertex shader
	for(int i = 0; i < shapes.size(); i++){
		//printf("%s\n", shapes[i].name.c_str());
		if (useMaterial) {
			m_shader->SetUniform("surfaceDiffuse", glm::vec4(shapes[i].material.diffuse[0], shapes[i].material.diffuse[1], shapes[i].material.diffuse[2], 1.0));
			m_shader->SetUniform("surfaceSpecular", glm::vec4(shapes[i].material.specular[0], shapes[i].material.specular[1], shapes[i].material.specular[2], 1.0));
			//m_shader->SetUniform("surfaceAmbient", glm::vec4(0, 0, 0, 0));

			m_shader->SetUniform("surfaceAmbient", glm::vec4(shapes[i].material.ambient[0], shapes[i].material.ambient[1], shapes[i].material.ambient[2], 1.0));
			m_shader->SetUniform("shininess", shapes[i].material.shininess);
			m_shader->SetUniform("surfaceEmissive", glm::vec4(shapes[i].material.emission[0], shapes[i].material.emission[1], shapes[i].material.emission[2], 1.0));
			//printf("Shininess: %f\n", shapes[i].material.shininess);
		}
		else {
			m_shader->SetUniform("surfaceDiffuse", glm::vec4(1, 1, 1, 1));
			m_shader->SetUniform("surfaceSpecular", glm::vec4(1, 1, 1, 1));
			m_shader->SetUniform("surfaceAmbient", glm::vec4(1, 1, 1, 1));
			m_shader->SetUniform("shininess", 1.0f);
			m_shader->SetUniform("surfaceEmissive", glm::vec4(1, 1, 1, 1));
			//printf("Shininess: 1\n");
		}
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO[i]); // Bind VBO.
		glEnableVertexAttribArray((*m_shader)["vertexPosition"]); // Enable vertex attribute.
		glVertexAttribPointer((*m_shader)["vertexPosition"], 3, GL_FLOAT, GL_FALSE, sizeof(float)*3, 0); // Attribute pointer.
		glBindBuffer(GL_ARRAY_BUFFER, m_NBO[i]); // Bind NBO.
		glEnableVertexAttribArray((*m_shader)["vertexNormal"]); // Enable normal attribute.
		glVertexAttribPointer((*m_shader)["vertexNormal"], 3, GL_FLOAT, GL_FALSE, sizeof(float)*3, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO[i]); // Bind IBO.
		glDrawElements(GL_TRIANGLES, shapes[i].mesh.indices.size(), GL_UNSIGNED_INT, 0); // Draw using indices
	}
	m_shader->DeActivate(); // Unbind shader.

}


/*When a new vertex is added, you need to tell the VBO about it.*/
void Model::updateBuffers()
{
	for(int i =0; i<shapes.size();i++){
		
		// Tell OpenGL which VBO you want to work on right now
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO[i]);
		// Fillthe VBO with vertex data.
		glBufferData(GL_ARRAY_BUFFER, shapes[i].mesh.positions.size() * sizeof(float), &shapes[i].mesh.positions[0], GL_STATIC_DRAW);
		// Tell OpenGL which VBO you want to work on right now
		glBindBuffer(GL_ARRAY_BUFFER, m_NBO[i]);
		// Fillthe VBO with vertex data.
		glBufferData(GL_ARRAY_BUFFER, shapes[i].mesh.normals.size() * sizeof(float), &shapes[i].mesh.normals[0], GL_STATIC_DRAW);
		// Bind ibo to the index buffer.
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO[i]);
		// Fill index buffer with index data.
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, shapes[i].mesh.indices.size() * sizeof(unsigned int), &shapes[i].mesh.indices[0], GL_STATIC_DRAW);
		
	}

}


/*Finds the max's and the min's of a models vertices*/
void Model::findBounds()
{
	float minX = 0, minY = 0, minZ = 0, maxX = 0, maxY = 0, maxZ = 0; // Initializing max and min values to 0 (float)
	//std::cout << shapes[0].mesh.positions.size();
	// Loop through all vertex positions and record max + min
	// Iterates 3 positions at a time due to the nature of the data
	for (int i = 0; i < shapes.size(); i++)
	{
		for (int j = 0; j < shapes[i].mesh.positions.size(); j += 3) {

			// On initial loop ([0]), set all appropriate min and max values to the first x, y, z positions
			if (i == 0 && j == 0)
			{
				minX = shapes[i].mesh.positions[j];
				minY = shapes[i].mesh.positions[j + 1];
				minZ = shapes[i].mesh.positions[j + 2];
				maxX = shapes[i].mesh.positions[j];
				maxY = shapes[i].mesh.positions[j + 1];
				maxZ = shapes[i].mesh.positions[j + 2];
				continue;
			}
			// When a new max/min is found, replace with current appropriate xyz value
			if (minX > shapes[i].mesh.positions[j])
				minX = shapes[i].mesh.positions[j];
			if (minY > shapes[i].mesh.positions[j + 1])
				minY = shapes[i].mesh.positions[j + 1];
			if (minZ > shapes[i].mesh.positions[j + 2])
				minZ = shapes[i].mesh.positions[j + 2];
			if (maxX < shapes[i].mesh.positions[j])
				maxX = shapes[i].mesh.positions[j];
			if (maxY < shapes[i].mesh.positions[j + 1])
				maxY = shapes[i].mesh.positions[j + 1];
			if (maxZ < shapes[i].mesh.positions[j + 2])
				maxZ = shapes[i].mesh.positions[j + 2];
		}
	}

	// Creating an array of the found min/max values to pass to setBounds
	boxVerts[0] = minX;
	boxVerts[1] = minY;
	boxVerts[2] = minZ;
	boxVerts[3] = maxX;
	boxVerts[4] = maxY;
	boxVerts[5] = maxZ;

	setVertices(boxVerts);
	if (isPlayer)
		setPoints();
}


/*Sets vertices for the bounding box of a model*/
void Model::setVertices(float values[6])
{
	bool bX = true, bY = true, bZ = true;	// Booleans used for logic, to follow a specific pattern
	for (int i = 0; i < 24; i++)
	{
		// The first 4 x positions will be minX, afterwards maxX
		if (i % 3 == 0) {
			if (bX == true)
				varray[i] = values[0];
			else
				varray[i] = values[3];
			if (i == 9)
				bX = false;
		}
		// The first two y positions will be minY and then switch to maxY, this process repeats every two unique positions
		else if ((i + 2) % 3 == 0) {
			if (bY == true)
				varray[i] = values[1];
			else
				varray[i] = values[4];
			if ((i + 2) % 6 == 0) {
				if (bY == true)
					bY = false;
				else bY = true;
			}
		}
		// The first z position is minZ and the next is maxZ, this process repeats
		else {
			if (bZ == true) {
				varray[i] = values[2];
				bZ = false;
			}
			else {
				varray[i] = values[5];
				bZ = true;
			}
		}
	}
}


void Model::setPoints()
{
	glm::vec3 offset;

	properties.position1 = glm::vec3(boxVerts[0], boxVerts[1], boxVerts[2]);
	properties.position2 = glm::vec3(boxVerts[3], boxVerts[1], boxVerts[5]);
	properties.position3 = glm::vec3(boxVerts[3], boxVerts[1], boxVerts[2]);
	properties.position4 = glm::vec3(boxVerts[0], boxVerts[1], boxVerts[5]);


	// Top edge
	offset = (properties.position3 - properties.position1) / 5.0f;
	properties.points[0] = properties.position1;
	properties.points[1] = properties.position1 + offset;
	properties.points[2] = properties.position1 + offset * 2.0f;
	properties.points[3] = properties.position1 + offset * 3.0f;

	// Right edge
	offset = (properties.position2 - properties.position3) / 5.0f;
	properties.points[4] = properties.position3;
	properties.points[5] = properties.position3 + offset;
	properties.points[6] = properties.position3 + offset * 2.0f;
	properties.points[7] = properties.position3 + offset * 3.0f;

	// Bottom edge
	offset = (properties.position4 - properties.position2) / 5.0f;
	properties.points[8] = properties.position2;
	properties.points[9] = properties.position2 + offset;
	properties.points[10] = properties.position2 + offset * 2.0f;
	properties.points[11] = properties.position2 + offset * 3.0f;

	// Left edge
	offset = (properties.position1 - properties.position4) / 5.0f;
	properties.points[12] = properties.position4;
	properties.points[13] = properties.position4 + offset;
	properties.points[14] = properties.position4 + offset * 2.0f;
	properties.points[15] = properties.position4 + offset * 3.0f;
}

/*Attaches the VBO and IBO for the bounding box to the buffers*/
void Model::updateBuffersBB()
{
	for (int i = 0; i < shapes.size();i++) {

		// Tell OpenGL which VBO you want to work on right now (bounding box)
		glBindBuffer(GL_ARRAY_BUFFER, m_VBOBB[i]);
		// Fill the VBO with bounding box vertex data.
		glBufferData(GL_ARRAY_BUFFER, sizeof(varray), varray, GL_STATIC_DRAW);
		// Bind ibo to the index bounding box buffer.
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBOBB[i]);
		// Fill index buffer with index data of the bounding box.
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	}
}


/*
Render the bounding box to the screen
@param Modelview - the model view matrix that defines where the camera is looking
@param Projection - the projection matrix that defines how 3D vertices are projected on the 2D screen.
*/
void Model::renderBB(glm::mat4 ModelView, glm::mat4 Projection)
{
	m_shaderBB->Activate(); // Bind shader to the specific BB shader

	//update the variables in the shader program
	m_shaderBB->SetUniform("Projection", Projection); // send projection to vertex shader
	m_shaderBB->SetUniform("ModelView", ModelView);  // send modelview to vertex shader
	m_shaderBB->SetUniform("lightPosition", glm::vec4(1.0, 0.0, 0.0, 1.0)); // send light position to vertex shader
	for (int i = 0; i < shapes.size(); i++) {
		glBindBuffer(GL_ARRAY_BUFFER, m_VBOBB[i]); // Bind VBO.
		glEnableVertexAttribArray((*m_shaderBB)["vertexPosition"]); // Enable vertex attribute.
		glVertexAttribPointer((*m_shaderBB)["vertexPosition"], 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0); // Attribute pointer.
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBOBB[i]); // Bind IBO.
		glDrawElements(GL_LINES, sizeof(indices), GL_UNSIGNED_INT, 0); // Draw using indices + GL_LINES
	}
	m_shaderBB->DeActivate(); // Unbind shader.
}

void Model::setProperties(glm::mat4 pos)
{
	properties.position1 = glm::vec3(pos * glm::vec4(boxVerts[0], boxVerts[1], boxVerts[2], 1));
	properties.position2 = glm::vec3(pos * glm::vec4(boxVerts[3], boxVerts[4], boxVerts[5], 1));

	if (isPlayer)
	{
		properties.position3 = glm::vec3(pos * glm::vec4(boxVerts[3], boxVerts[1], boxVerts[2], 1));
		properties.position4 = glm::vec3(pos * glm::vec4(boxVerts[0], boxVerts[1], boxVerts[5], 1));

		for (int i = 0; i < 16; i++)
		{
			properties.positions[i] = glm::vec3(pos * glm::vec4(properties.points[i].x, properties.points[i].y, properties.points[i].z, 1.0));
		}
	}
}
