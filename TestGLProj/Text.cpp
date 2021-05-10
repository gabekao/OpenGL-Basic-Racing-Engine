#include "Text.h"


Text::Text(Shader *shader, std::string font)
{
	t_shader = shader;

	LoadFontTextures(font);
	SetBuffers();
}

void Text::LoadFontTextures(std::string font)
{
	FT_Library ft;

	FT_Init_FreeType(&ft);

	if (font.empty())
	{
		std::cout << "ERROR: Failed to load font file" << std::endl;
		return;
	}

	FT_Face face;

	if (FT_New_Face(ft, font.c_str(), 0, &face))
	{
		std::cout << "ERROR: Failed to load font" << std::endl;
		return;
	}
	else
	{
		FT_Set_Pixel_Sizes(face, 0, 48);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		for (unsigned char c = 0; c < 128; c++)
		{
			if (FT_Load_Char(face, c, FT_LOAD_RENDER))
			{
				std::cout << "ERROR unable to load glyph" << std::endl;
				continue;
			}

			unsigned int texture;
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);
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

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			Character character = {
				texture,
				glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
				glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
				static_cast<unsigned int>(face->glyph->advance.x)
			};

			Characters.insert(std::pair<char, Character>(c, character));
		}
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	FT_Done_Face(face);
	FT_Done_FreeType(ft);
}

void Text::SetBuffers()
{
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void Text::RenderText(std::string text, float x, float y, float scale, glm::vec3 color)
{
	
	glm::mat4 projection = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f);
	t_shader->Activate();
	t_shader->SetUniform("textColor", color);
	t_shader->SetUniform("projection", projection);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(VAO);

	std::string::const_iterator c;

	for (c = text.begin(); c != text.end(); c++)
	{
		Character ch = Characters[*c];

		float xpos = x + ch.Bearing.x * scale;
		float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

		float w = ch.Size.x * scale;
		float h = ch.Size.y * scale;

		float vertices[6][4] = {
			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos,     ypos,       0.0f, 1.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },

			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },
			{ xpos + w, ypos + h,   1.0f, 0.0f }
		};
		

		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

		glBindBuffer(GL_ARRAY_BUFFER, 0);


		glDrawArrays(GL_TRIANGLES, 0, 6);
		

		x += (ch.Advance >> 6) * scale;
	}

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	
}
