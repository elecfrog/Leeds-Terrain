#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <filesystem>
#include <unordered_map>


class Shader
{
public:

	unsigned int ID;

	const char* vertSource;
	const char* tescSource;
	const char* teseSource;
	const char* geomSource;
	const char* fragSource;

	Shader() = default;

	Shader(const char* vertex_file_path, const char* fragment_file_path, const char* tess_control_path = nullptr, const char* tess_eval_file_path = nullptr, const char* geometry_file_path = nullptr)
	{
		vertSource = vertex_file_path;
		fragSource = fragment_file_path;
		tescSource = tess_control_path;
		teseSource = tess_eval_file_path;
		geomSource = geometry_file_path;
		LoadShaders(vertSource, fragSource, tescSource, teseSource, geomSource);
	}

	// Cleanup Shader
	~Shader()
	{
		glDeleteProgram(this->ID);
	}

private:
	// Read and Compile Shader
	bool readAndCompileShader(const char* shader_path, const unsigned int& id) 
	{
		// Read the Vertex Shader code from the file
		std::string VertexShaderCode;
		std::ifstream VertexShaderStream(shader_path, std::ios::in);
		if (VertexShaderStream.is_open()) {
			std::stringstream sstr;
			sstr << VertexShaderStream.rdbuf();
			VertexShaderCode = sstr.str();
			VertexShaderStream.close();
		}
		else {
			printf("Impossible to open %s. Are you in the right directory?\n", shader_path);
			return false;
		}

		// Compile Vertex Shader
		printf("Compiling shader : %s\n", shader_path);
		char const* VertexSourcePointer = VertexShaderCode.c_str();
		glShaderSource(id, 1, &VertexSourcePointer, NULL);
		glCompileShader(id);

		GLint Result = GL_FALSE;
		int InfoLogLength;

		// Check  Shader
		glGetShaderiv(id, GL_COMPILE_STATUS, &Result);
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &InfoLogLength);
		if (InfoLogLength > 0) {
			std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
			glGetShaderInfoLog(id, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
			printf("%s\n", &VertexShaderErrorMessage[0]);
		}
		std::cout << "Compilation of Shader: " << shader_path << " " << (Result == GL_TRUE ? "Success" : "Failed!") << std::endl;
		return Result == 1;
	}

public:
	void Bind() 
	{
		glUseProgram(this->ID);
	}

	void UnBind()
	{
		glUseProgram(0);
	}
	// Link a Shader, Using static shader to use.
	bool LoadShaders(const char* vertex_file_path, const char* fragment_file_path, const char* tess_control_path = nullptr, const char* tess_eval_file_path = nullptr, const char* geometry_file_path = nullptr)
	{
		// Create the shaders - tasks 1 and 2 
		GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
		GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

		//Create the shaders - task 3
		GLuint TesselationControlShaderID = 0;
		GLuint TesselationEvalShaderID = 0;

		//Create the shader - task 4
		GLuint GeometryShaderID = 0;

		readAndCompileShader(vertex_file_path, VertexShaderID);
		readAndCompileShader(fragment_file_path, FragmentShaderID);

		if (tess_control_path && tess_eval_file_path) {
			TesselationControlShaderID = glCreateShader(GL_TESS_CONTROL_SHADER);
			TesselationEvalShaderID = glCreateShader(GL_TESS_EVALUATION_SHADER);
			readAndCompileShader(tess_control_path, TesselationControlShaderID);
			readAndCompileShader(tess_eval_file_path, TesselationEvalShaderID);
		}

		if (geometry_file_path) {
			GeometryShaderID = glCreateShader(GL_GEOMETRY_SHADER);
			readAndCompileShader(geometry_file_path, GeometryShaderID);
		}

		GLint Result = GL_FALSE;
		int InfoLogLength;

		// Link the program
		printf("Linking program\n");
		this->ID = glCreateProgram();
		glAttachShader(this->ID, VertexShaderID);
		glAttachShader(this->ID, FragmentShaderID);

		if (tess_control_path && tess_eval_file_path) {
			glAttachShader(this->ID, TesselationControlShaderID);
			glAttachShader(this->ID, TesselationEvalShaderID);
		}

		if (geometry_file_path)
			glAttachShader(this->ID, GeometryShaderID);


		glLinkProgram(this->ID);

		// Check the program
		glGetProgramiv(this->ID, GL_LINK_STATUS, &Result);
		glGetProgramiv(this->ID, GL_INFO_LOG_LENGTH, &InfoLogLength);
		if (InfoLogLength > 0) {
			std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
			glGetProgramInfoLog(this->ID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
			printf("%s\n", &ProgramErrorMessage[0]);
		}
		std::cout << "Linking program: " << (Result == GL_TRUE ? "Success" : "Failed!") << std::endl;


		glDeleteShader(VertexShaderID);
		glDeleteShader(FragmentShaderID);
		if (TesselationControlShaderID != 0 && TesselationEvalShaderID != 0) {
			glDeleteShader(TesselationControlShaderID);
			glDeleteShader(TesselationEvalShaderID);
		}
		if (GeometryShaderID != 0) {
			glDeleteShader(GeometryShaderID);
		}

		return true;
	}

};

