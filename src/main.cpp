// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

// Include GLM customized 3C file
#include <common/controls.hpp>

#include "BMPLoader.hpp"

// Using Texture Class Instead of Writing A lot of OpenGL Sentences
#include "Texture.hpp"
#include "Shader.hpp"

// Init Width and Height of the window
static constexpr int window_width = 1920;
static constexpr int window_height = 1080;

//
static constexpr int n_points = 200;
static constexpr float m_scale = 0.5f;

//Variables

// GLFW Window Object
GLFWwindow* window;

//Model
std::vector<unsigned int> indices;
std::vector<glm::vec3> vertices;
std::vector<glm::vec2> uvs;
std::vector<glm::vec3> normals;

// VAO
GLuint VertexArrayID;

// Buffers for VAO
GLuint vertexbuffer;
GLuint uvbuffer;
GLuint normalbuffer;
GLuint elementbuffer;

// Load OBJ files from Hard Disk
bool loadOBJ(const char* path, std::vector<glm::vec3>& out_vertices, std::vector<glm::vec2>& out_uvs, std::vector<glm::vec3>& out_normals, std::vector<unsigned int>& out_indices) {
	printf("Loading OBJ file %s...\n", path);

	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec3> temp_vertices;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;


	FILE* file = fopen(path, "r");
	if (file == NULL) {
		printf("Impossible to open the file ! Are you in the right path ?\n");
		return false;
	}

	while (1) {

		char lineHeader[128];
		// read the first word of the line
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

		// else : parse lineHeader

		if (strcmp(lineHeader, "v") == 0) {
			glm::vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			temp_vertices.push_back(vertex);
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			glm::vec2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y);
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			glm::vec3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			temp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0) {
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			if (matches != 9) {
				printf("File can't be read by our simple parser :-( Try exporting with other options\n");
				fclose(file);
				return false;
			}

			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			uvIndices.push_back(uvIndex[0]);
			uvIndices.push_back(uvIndex[1]);
			uvIndices.push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);

		}
		else {
			// Probably a comment, eat up the rest of the line
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file);
		}

	}

	// For each vertex of each triangle
	for (unsigned int i = 0; i < vertexIndices.size(); i++) {

		// Get the indices of its attributes
		unsigned int vertexIndex = vertexIndices[i];
		unsigned int uvIndex = uvIndices[i];
		unsigned int normalIndex = normalIndices[i];

		// Get the attributes thanks to the index
		glm::vec3 vertex = temp_vertices[vertexIndex - 1];
		glm::vec2 uv = temp_uvs[uvIndex - 1];
		glm::vec3 normal = temp_normals[normalIndex - 1];

		// Put the attributes in buffers
		out_vertices.push_back(vertex);
		out_uvs.push_back(uv);
		out_normals.push_back(normal);
		out_indices.push_back(i);
	}
	fclose(file);
	return true;
}

// initialize GLFW & GLEW with Basic Information
int initializeGLFW()
{
	// Initialize GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 1);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(window_width, window_height, "OpenGLRenderer", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible.\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}


	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	// Hide the mouse and enable unlimited mouvement
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwPollEvents();
	glfwSetCursorPos(window, window_width / 2, window_height / 2);

	return 0;
}

// Load Mesh Info from Path. And Bind with VAO, VBO
void LoadModel(std::string path, GLint mode)
{

	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// If path is empty, using internal points. else Load Obj from Disk
	if (path == "")
	{
		// Create mesh of n_points x n_points with normals up, and obvious uv mapping.
		// If path is an empty, Just Load a implicit Plane with length of n_points
		for (int i = 0; i < n_points; i++)
		{
			for (int j = 0; j < n_points; j++)
			{
				//Lets center the plane around the zero
				float x = (m_scale * i) - (m_scale * n_points) / 2.0f;
				float z = (m_scale * j) - (m_scale * n_points) / 2.0f;
				vertices.push_back(glm::vec3(x, 0, z));
				uvs.push_back(glm::vec2(float(i + 0.5f) / float(n_points - 1), float(j + 0.5f) / float(n_points - 1)));
				normals.push_back(glm::vec3(0, 1, 0));

			}
		}
		if (mode == GL_TRIANGLES) {
			//now do a triangle strip
			int n = 0;
			for (int i = 0; i < n_points; i++)
			{
				for (int j = 0; j < n_points; j++)
				{
					if (j != n_points - 1 && i != n_points - 1)
					{
						int topLeft = n;
						int topRight = topLeft + 1;
						int bottomLeft = topLeft + n_points;
						int bottomRight = bottomLeft + 1;
						indices.push_back(topLeft);
						indices.push_back(topRight);
						indices.push_back(bottomLeft);
						indices.push_back(bottomLeft);
						indices.push_back(topRight);
						indices.push_back(bottomRight);
					}
					n++;

				}
			}
		}
		else if (mode == GL_PATCHES)
		{
			// Now Do a TCS strip
			int n = 0;
			for (int i = 0; i < n_points; i++)
			{
				for (int j = 0; j < n_points; j++)
				{
					if (j != n_points - 1 && i != n_points - 1)
					{
						int topLeft = n;
						int topRight = topLeft + 1;
						int bottomLeft = topLeft + n_points;
						int bottomRight = bottomLeft + 1;
						// CW
						indices.push_back(topLeft);
						indices.push_back(topRight);
						indices.push_back(bottomRight);
						indices.push_back(bottomLeft);
					}
					n++;

				}
			}
		}
		else {
			std::cout << "Can't process that mode..." << std::endl;
			return;
		}
	}
	else 
	{
		loadOBJ(path.c_str(), vertices, uvs, normals, indices);
	}


	// Load it into a VBO

	glEnableVertexAttribArray(0);
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
	glVertexAttribPointer(
		0,                  // attribute
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);

	glEnableVertexAttribArray(1);
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);
	glVertexAttribPointer(
		1,                                // attribute
		2,                                // size
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
	);

	glEnableVertexAttribArray(2);
	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
	glVertexAttribPointer(
		2,                                // attribute
		3,                                // size
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
	);



	// Generate a buffer for the indices as well
	glGenBuffers(1, &elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
}

// Cleanup VBO and shader
void UnloadModel()
{
	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &uvbuffer);
	glDeleteBuffers(1, &normalbuffer);
	glDeleteBuffers(1, &elementbuffer);
	glDeleteVertexArrays(1, &VertexArrayID);
}

// Main Function | Rendering Loop
int main(void)
{
	// Initialize and create a window.
	if (initializeGLFW() != 0) return -1;

 	// Gray background
	glClearColor(0.7f, 0.8f, 1.0f, 0.0f);
	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);
	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);
	
	glPatchParameteri(GL_PATCH_VERTICES, 4); // Quads
	
	// Use my customized shader Class
	Shader terrainShader("Terrain.vert", "Terrain.frag", "Terrain.tesc", "Terrain.tese");
	Shader elecfrogShader("Flower.vert", "Flower.frag", nullptr, nullptr,"Flower.geom");
	//Shader elecfrogShader("Flower.vert", "Flower.frag");

	// Use my customized Texture Class
	std::vector<Texture*> textures;
	// height map
	textures.emplace_back(new Texture("mountains_height.bmp", GL_NEAREST));
	// diffuse
	textures.emplace_back(new Texture("rocks.bmp"));
	textures.emplace_back(new Texture("grass.bmp"));
	textures.emplace_back(new Texture("snow.bmp"));	
	// specular	
	textures.emplace_back(new Texture("rocks-s.bmp"));
	textures.emplace_back(new Texture("grass-s.bmp"));
	textures.emplace_back(new Texture("snow-s.bmp"));

	//LoadModel("banana.obj", GL_TRIANGLES);


	// Load an empty string to show the texture, Using Patch
	LoadModel("", GL_PATCHES);

	// Our light position is fixed
	glm::vec3 lightPos = glm::vec3(0, -10.5, -0.5);
//	glm::vec3 lightPos = glm::vec3(0, 4, 4);
	bool n = false;
	bool reloadShaders = false;
	// For speed computation
	double lastTime = glfwGetTime();
	int nbFrames = 0;
	do {
		
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			reloadShaders = true;
		}
		if (reloadShaders && glfwGetKey(window, GLFW_KEY_S) == GLFW_RELEASE) {
			terrainShader.~Shader();
			elecfrogShader.~Shader();
			//LoadShaders(programID, vertShader, fragShader, tescShader, teseShader);
			terrainShader.LoadShaders(terrainShader.vertSource, terrainShader.fragSource, terrainShader.tescSource, terrainShader.teseSource);
			elecfrogShader.LoadShaders(elecfrogShader.vertSource, elecfrogShader.fragSource,nullptr,nullptr, elecfrogShader.geomSource);
			reloadShaders = false;
		}
		

		// Measure speed
		double currentTime = glfwGetTime();
		nbFrames++;
		if (currentTime - lastTime >= 1.0) { // If last prinf() was more than 1sec ago
			// printf and reset
			printf("%f ms/frame\n", 1000.0 / double(nbFrames));
			nbFrames = 0;
			lastTime += 1.0;
		}

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Compute the MVP matrix from keyboard and mouse input
		computeMatricesFromInputs();
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();
		glm::mat4 ModelMatrix = glm::mat4(1.0);
		glm::mat4 ModelViewMatrix = ViewMatrix * ModelMatrix;
		glm::mat3 ModelView3x3Matrix = glm::mat3(ModelViewMatrix);
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

		// First pass: Base mesh
		terrainShader.Bind();

		// Set Mountain Hight Map
		textures[0]->Active(0);
		textures[0]->SetShaderUniform(glGetUniformLocation(terrainShader.ID, "DiffuseTextureSampler"));

		// Set Three Types of Diffuse textures: Rock Grass and Snow
		textures[1]->Active(1);
		textures[1]->SetShaderUniform(glGetUniformLocation(terrainShader.ID, "rt.rock"));

		textures[2]->Active(2);
		textures[2]->SetShaderUniform(glGetUniformLocation(terrainShader.ID, "rt.grass"));

		textures[3]->Active(3);
		textures[3]->SetShaderUniform(glGetUniformLocation(terrainShader.ID, "rt.snow"));
		
		// Set Three Types of Specular textures: Rock Grass and Snow
		textures[4]->Active(4);
		textures[4]->SetShaderUniform(glGetUniformLocation(terrainShader.ID, "rt.rock_s"));

		textures[5]->Active(5);
		textures[5]->SetShaderUniform(glGetUniformLocation(terrainShader.ID, "rt.grass_s"));

		textures[6]->Active(6);
		textures[6]->SetShaderUniform(glGetUniformLocation(terrainShader.ID, "rt.snow_s"));

		// Get a handle for our uniforms
		GLuint MatrixID = glGetUniformLocation(terrainShader.ID, "MVP");
		GLuint ViewMatrixID = glGetUniformLocation(terrainShader.ID, "V");
		GLuint ModelMatrixID = glGetUniformLocation(terrainShader.ID, "M");
		GLuint ModelView3x3MatrixID = glGetUniformLocation(terrainShader.ID, "MV3x3");
		GLuint LightID = glGetUniformLocation(terrainShader.ID, "LightPosition_worldspace");

		// Send our transformation to the currently bound shader, 
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);
		glUniformMatrix3fv(ModelView3x3MatrixID, 1, GL_FALSE, &ModelView3x3Matrix[0][0]);
		
		// Set the light position
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

		// KEY W Wire frame Mode
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) 
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else 
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		//Draw the triangles !
		glDrawElements(
			GL_PATCHES,      // mode
			(GLsizei)indices.size(),    // count
			GL_UNSIGNED_INT, // type
			(void*)0           // element array buffer offset
		);

		terrainShader.UnBind();

		elecfrogShader.Bind();

		// Set Mountain Hight Map
		textures[0]->Active(0);
		textures[0]->SetShaderUniform(glGetUniformLocation(elecfrogShader.ID, "DiffuseTextureSampler"));
		// Get a handle for our uniforms
		MatrixID = glGetUniformLocation(elecfrogShader.ID, "MVP");
		ViewMatrixID = glGetUniformLocation(elecfrogShader.ID, "V");
		unsigned int ProjectionMatrixID = glGetUniformLocation(elecfrogShader.ID, "P");
		ModelMatrixID = glGetUniformLocation(elecfrogShader.ID, "M");
		ModelView3x3MatrixID = glGetUniformLocation(elecfrogShader.ID, "MV3x3");
		LightID = glGetUniformLocation(elecfrogShader.ID, "LightPosition_worldspace");

		// Send our transformation to the currently bound shader, 
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);		
		glUniformMatrix4fv(ProjectionMatrixID, 1, GL_FALSE, &ProjectionMatrix[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);
		glUniformMatrix3fv(ModelView3x3MatrixID, 1, GL_FALSE, &ModelView3x3Matrix[0][0]);

		// Set the light position
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

		// KEY W Wire frame Mode
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		//Draw the triangles !
		glDrawElements(
			GL_PATCHES,      // mode
			(GLsizei)indices.size(),    // count
			GL_UNSIGNED_INT, // type
			(void*)0           // element array buffer offset
		);


		elecfrogShader.UnBind();

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} 
	// Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);


	UnloadModel();
	//UnloadTextures();
	for (const auto& t : textures)
	{
		t->~Texture();
	}
	terrainShader.~Shader();

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}
