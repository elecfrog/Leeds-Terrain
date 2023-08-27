#pragma once
/*
	Encapsulate Some OpenGL Texture Methods to make Life Easier.
*/ 

#include "BMPLoader.hpp"

class Texture
{
private:
	unsigned int ID;
	unsigned int slot;

	const char* file_name;

	int texWidth;
	int texHeight;
public:
	Texture() = default;
	
	Texture(const char* _name)
	{
		this->file_name = _name;
		this->ID  = loadBMP_custom(file_name, GL_LINEAR_MIPMAP_LINEAR, GL_MIRRORED_REPEAT, texWidth, texHeight);
	}

	Texture(const char* _name, GLenum tex_option)
	{
		this->file_name = _name;
		this->ID = loadBMP_custom(file_name, tex_option, GL_MIRRORED_REPEAT, texWidth, texHeight);
	}

	~Texture()
	{
		glDeleteTextures(1, &this->ID);
	}

	void Active(unsigned int _slot)
	{
		slot = _slot;
		glActiveTexture(GL_TEXTURE0 + _slot);
		glBindTexture(GL_TEXTURE_2D, this->ID);
	}

	void SetShaderUniform(unsigned int texLocationID, unsigned int _slot) const
	{
		glUniform1i(texLocationID, _slot);
	}

	void SetShaderUniform(unsigned int texLocationID) const
	{
		glUniform1i(texLocationID, this->slot);
	}

	void Unbind() const;

	int GetWidth() const { return texWidth; }
	int GetHeight() const { return texHeight; }
};