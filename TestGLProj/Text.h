#include <iostream>
#include <map>
#include <string>

#include <GL/glew.h>
#include <GL/GL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#define GL_CLAMP_TO_EDGE	0x812F

class Text {
public:
	
	Text(Shader *shader, std::string font);

	void RenderText(std::string text, float x, float y, float scale, glm::vec3 color);

private:
	
	struct Character {
		unsigned int TextureID;
		glm::ivec2 Size;
		glm::ivec2 Bearing;
		unsigned int Advance;
	};

	std::map<GLchar, Character> Characters;
	unsigned int VAO, VBO;
	Shader *t_shader;

	void LoadFontTextures(std::string font);
	void SetBuffers();
};