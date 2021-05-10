#include "Text.h"

/*Initializes text class by loading font textures and setting up buffers*/
Text::Text(Shader *shader, std::string font)
{
	// Shader being used
	t_shader = shader;

	// Load font textures
	LoadFontTextures(font);

	// Create buffers
	SetBuffers();
}

/*Uses FT library to load font glyphs*/
void Text::LoadFontTextures(std::string font)
{
	// FT libary
	FT_Library ft;

	// Initialize library
	FT_Init_FreeType(&ft);

	// Check to see if font exists
	if (font.empty())
	{
		std::cout << "ERROR: Failed to load font file" << std::endl;
		return;
	}

	// Create an FT face
	FT_Face face;

	// If unable to add new face return, else proceed to adding texture
	if (FT_New_Face(ft, font.c_str(), 0, &face))
	{
		std::cout << "ERROR: Failed to load font" << std::endl;
		return;
	}
	else
	{
		// Set pixel size of face
		FT_Set_Pixel_Sizes(face, 0, 48);

		// Unpack texture pixels
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		// Loop for 128 unique characters
		for (unsigned char c = 0; c < 128; c++)
		{
			// Use FT load char to load char into face
			if (FT_Load_Char(face, c, FT_LOAD_RENDER))
			{
				std::cout << "ERROR unable to load glyph" << std::endl;
				continue;
			}

			// Create texture id and bind 2D texture
			unsigned int texture;
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);

			// Assign texture properties
			glTexImage2D(
				GL_TEXTURE_2D,
				0,
				GL_RED,
				face->glyph->bitmap.width,
				face->glyph->bitmap.rows,
				0,
				GL_RED,
				GL_UNSIGNED_BYTE,
				face->glyph->bitmap.buffer);

			// Set texture paramaters
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			// Create a character class to insert into our character mapping
			Character character = {
				texture,
				glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
				glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
				static_cast<unsigned int>(face->glyph->advance.x)
			};

			// Insert new character into mapping based on current char
			Characters.insert(std::pair<char, Character>(c, character));
		}
		// Bind texture to 0 outside loop
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	// Close FT and face
	FT_Done_Face(face);
	FT_Done_FreeType(ft);
}

/*Sets up buffers for characters*/
void Text::SetBuffers()
{
	// Generate VAO and VBO
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	// Bind buffers
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	// Initialize data
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

/*Renders passed text string*/
void Text::RenderText(std::string text, float x, float y, float scale, glm::vec3 color)
{
	// Enable blending to display text
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Create a predefined projection to pass onto shader
	glm::mat4 projection = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f);
	
	// Set initial shader properties
	t_shader->Activate();
	t_shader->SetUniform("textColor", color);
	t_shader->SetUniform("projection", projection);

	// Activate textures, binding to VAO
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(VAO);

	// Current char being looked at
	std::string::const_iterator c;

	// Loop through all passed text
	for (c = text.begin(); c != text.end(); c++)
	{
		// Create character structure based on provided char
		Character ch = Characters[*c];

		// Set character x and y positions
		float xpos = x + ch.Bearing.x * scale;
		float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

		// Multiply size values by scaling
		float w = ch.Size.x * scale;
		float h = ch.Size.y * scale;

		// Vertex mapping for character
		float vertices[6][4] = {
			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos,     ypos,       0.0f, 1.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },

			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },
			{ xpos + w, ypos + h,   1.0f, 0.0f }
		};
		
		// Bind mapped character to texture
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		
		// Bind to VBO and use vertex data
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// Draw character
		glDrawArrays(GL_TRIANGLES, 0, 6);
		
		// Character size indentation for text
		x += (ch.Advance >> 6) * scale;
	}

	// Set buffers and array to 0
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Disable all blending
	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);
}
