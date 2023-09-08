#include <stdlib.h> // necesare pentru citirea shader-elor
#include <stdio.h>
#include <math.h> 
#include <vector>

#include <GL/glew.h>

#define GLM_FORCE_CTOR_INIT 

#include <glfw3.h>

#include <iostream>
#include <fstream>
#include <sstream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#pragma comment (lib, "glfw3dll.lib")
#pragma comment (lib, "glew32.lib")
#pragma comment (lib, "OpenGL32.lib")

// settings
const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 900;

#include "Camera.h"
#include "Shader.h"
#include <ext/matrix_transform.hpp>
#include "FileSystem.h""

const GLchar* VertexShader =
{
   "#version 330\n"\
   "layout (location = 0) in vec3 aPos;\n"\
   "layout (location = 1) in vec3 aColor;\n"\
   "layout (location = 2) in vec2 aTexCoord;\n"\
   "out vec3 ourColor;\n"\
   "out vec2 TexCoord;\n"\
   "uniform mat4 ProjMatrix;\n"\
   "uniform mat4 ViewMatrix;\n"\
   "uniform mat4 WorldMatrix;\n"\
   "void main()\n"\
   "{\n"\
   "gl_Position = ProjMatrix * ViewMatrix * WorldMatrix * vec4(aPos, 1.0);\n"\
   "ourColor = aColor;\n"\
   "TexCoord = vec2(aTexCoord.x, aTexCoord.y);\n"\
   "}\n"
};

const GLchar* FragmentShader =
{
   "#version 330\n"\
   "out vec4 FragColor;\n"\
   "in vec3 ourColor;\n"\
   "in vec2 TexCoord;\n"\
   "uniform float mixValue;\n"\
   "uniform sampler2D texture1;\n"\
   "uniform sampler2D texture2;\n"\
   "void main()\n"\
   "{\n"\
   "  FragColor =   mix(texture(texture1, TexCoord), texture(texture2, TexCoord), mixValue);\n"\
   "}\n"
};

unsigned int VertexShaderId, FragmentShaderId, ProgramId;
GLuint ProjMatrixLocation, ViewMatrixLocation, WorldMatrixLocation;
unsigned int texture1Location, texture2Location, MixValueLocation;
float g_fMixValue = 0.5;

Camera* pCamera = nullptr;

void CreateShaders()
{
	VertexShaderId = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(VertexShaderId, 1, &VertexShader, NULL);
	glCompileShader(VertexShaderId);

	FragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(FragmentShaderId, 1, &FragmentShader, NULL);
	glCompileShader(FragmentShaderId);

	ProgramId = glCreateProgram();
	glAttachShader(ProgramId, VertexShaderId);
	glAttachShader(ProgramId, FragmentShaderId);
	glLinkProgram(ProgramId);

	GLint Success = 0;
	GLchar ErrorLog[1024] = { 0 };

	glGetProgramiv(ProgramId, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(ProgramId, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
		exit(1);
	}

	glValidateProgram(ProgramId);
	glGetProgramiv(ProgramId, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(ProgramId, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
		exit(1);
	}

	glUseProgram(ProgramId);

	ProjMatrixLocation = glGetUniformLocation(ProgramId, "ProjMatrix");
	ViewMatrixLocation = glGetUniformLocation(ProgramId, "ViewMatrix");
	WorldMatrixLocation = glGetUniformLocation(ProgramId, "WorldMatrix");

	MixValueLocation = glGetUniformLocation(ProgramId, "mixValue");
	glUniform1f(MixValueLocation, g_fMixValue);

	glUniform1i(glGetUniformLocation(ProgramId, "texture1"), 0);
	glUniform1i(glGetUniformLocation(ProgramId, "texture2"), 1);
}

void RenderSkybox(unsigned int skyboxVBO, unsigned int skyboxEBO)
{
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxEBO);

	int indexArraySize;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &indexArraySize);
	glDrawElements(GL_TRIANGLES, indexArraySize / sizeof(unsigned int), GL_UNSIGNED_INT, 0);
}

GLuint loadCubemap(std::vector<std::string> faces)
{
	//initialize texture id and bind it
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			//stbi_image_free(data);
		}
		else {
			std::cout << "Could not load texture at path: " << faces[i] << std::endl;
			//stbi_image_free(data);
		}
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

void Cleanup()
{
	delete pCamera;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// timing
double deltaTime = 0.0f;	// time between current frame and last frame
double lastFrame = 0.0f;

unsigned int CreateTexture(const std::string& strTexturePath);


void drawGrass(Shader& shaderBlending, glm::mat4& model, glm::vec3 pos)
{
	const float pi = 3.1415;
	std::vector<float> angles =
	{
		0.0f,
		pi / 4.0f,
		pi / 2.0f,
		3.0f * pi / 4.0f
	};

	for (const auto& angle : angles)
	{
		model = glm::rotate(model, angle, pos);
		shaderBlending.SetMat4("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
}


int main(int argc, char** argv)
{
	std::string strFullExeFileName = argv[0];
	std::string strSourcePath;
	const size_t last_slash_idx = strFullExeFileName.rfind('\\');
	if (std::string::npos != last_slash_idx) {
		strSourcePath = strFullExeFileName.substr(0, last_slash_idx);
		strSourcePath = strSourcePath.substr(0, strSourcePath.find("\\x64"));
		strSourcePath.append("\\KartMaster\\");
	}

	// glfw: initialize and configure
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// glfw window creation
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "KartMaster", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glewInit();

	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Skybox vertices
	float vertices[] = 
	{
		-10.0f, -10.0f, 10.0f,   10.0f, -10.0f, -10.0f,   1.0f, 1.0f,
		10.0f, -10.0f, 10.0f,   -10.0f, 10.0f, -10.0f,   1.0f, 0.0f,
		10.0f, 10.0f, 10.0f,   -10.0f, -10.0f, 10.0f,   0.0f, 0.0f,
		-10.0f, 10.0f, 10.0f,   10.0f, -10.0f, -10.0f,   0.0f, 1.0f,
		-10.0f, -10.0f, -10.0f,   -10.0f, 10.0f, -10.0f,   0.0f, 0.0f,
		10.0f, -10.0f, -10.0f,   -10.0f, -10.0f, 10.0f,   0.0f, 1.0f,
		10.0f, 10.0f, -10.0f,   10.0f, -10.0f, -10.0f,   1.0f, 1.0f,
		-10.0f, 10.0f, -10.0f,   -10.0f, 10.0f, -10.0f,   1.0f, 0.0f
	};
	unsigned int indices[] = 
	{
	   0,1,2,
	   0,2,3,
	   1,5,6,
	   1,6,2,
	   5,4,7,
	   5,7,6,
	   4,0,3,
	   4,3,7,
	   0,5,1,
	   0,4,5,
	   3,2,6,
	   3,6,7
	};
	// Skybox VAO si VBO
	unsigned int skyboxVAO, skyboxVBO, skyboxEBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glGenBuffers(1, &skyboxEBO);

	glBindVertexArray(skyboxVAO);

	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// Floor vertices
	float floorVertices[] =
	{
		// positions          // texture Coords 
		5.0f, -0.5f,  5.0f,  1.0f, 0.0f,
		-5.0f, -0.5f,  5.0f,  0.0f, 0.0f,
		-5.0f, -0.5f, -5.0f,  0.0f, 1.0f,

		5.0f, -0.5f,  5.0f,  1.0f, 0.0f,
		-5.0f, -0.5f, -5.0f,  0.0f, 1.0f,
		5.0f, -0.5f, -5.0f,  1.0f, 1.0f
	};

	// Floor VAO si VBO
	unsigned int floorVAO, floorVBO;
	glGenVertexArrays(1, &floorVAO);
	glGenBuffers(1, &floorVBO);
	glBindVertexArray(floorVAO);
	glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertices), &floorVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));


	// Grass vertices
	float grassVertices[] =
	{
		// positions          // texture Coords 
		0.5f,	0.5f,	0.0f,	1.0f,	0.0f,
		-0.5f,	0.5f,	0.0f,	0.0f,	0.0f,
		-0.5f,	-0.5f,	0.0f,	0.0f,	1.0f,

		0.5f,	0.5f,	0.0f,	1.0f,	0.0f,
		-0.5f,	-0.5f,	0.0f,	0.0f,	1.0f,
		0.5f,	-0.5f,	0.0f,	1.0f,	1.0f
	};

	// Grass VAO si VBO
	unsigned int grassVAO, grassVBO;
	glGenVertexArrays(1, &grassVAO);
	glGenBuffers(1, &grassVBO);
	glBindVertexArray(grassVAO);
	glBindBuffer(GL_ARRAY_BUFFER, grassVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(grassVertices), &grassVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	// Skybox texture
	const std::vector<std::string> skyboxFacesDay{
			FileSystem::getPath("Textures/skybox/day/right.png"),
			FileSystem::getPath("Textures/skybox/day/left.png"),
			FileSystem::getPath("Textures/skybox/day/top.png"),
			FileSystem::getPath("Textures/skybox/day/bottom.png"),
			FileSystem::getPath("Textures/skybox/day/front.png"),
			FileSystem::getPath("Textures/skybox/day/back.png")
	};
	const std::vector<std::string> skyboxFacesNight{
			FileSystem::getPath("Textures/skybox/night/right.png"),
			FileSystem::getPath("Textures/skybox/night/left.png"),
			FileSystem::getPath("Textures/skybox/night/top.png"),
			FileSystem::getPath("Textures/skybox/night/bottom.png"),
			FileSystem::getPath("Textures/skybox/night/front.png"),
			FileSystem::getPath("Textures/skybox/night/back.png")
	};

	unsigned int skyboxTexture = loadCubemap(skyboxFacesDay);

	// Floor texture
	unsigned int floorTexture = CreateTexture(strSourcePath + "Textures\\track.jpg");

	// Grass texture
	unsigned int grassTexture = CreateTexture(strSourcePath + "Textures\\car.png");

	// Create camera
	pCamera = new Camera(SCR_WIDTH, SCR_HEIGHT, glm::vec3(0.0, 0.0, 3.0));
	Shader shaderSkybox((strSourcePath + "Shaders\\skybox.vs").c_str(),
		(strSourcePath + "Shaders\\skybox.fs").c_str());
	Shader shaderFloor((strSourcePath + "Shaders\\Floor.vs").c_str(),
		(strSourcePath + "Shaders\\Floor.fs").c_str());
	Shader shaderBlending((strSourcePath + "Shaders\\Blending.vs").c_str(),
		(strSourcePath + "Shaders\\Blending.fs").c_str());
	shaderBlending.SetInt("texture1", 0);
	GLuint ProjMatrixLocation, ViewMatrixLocation, WorldMatrixLocation;
	ProjMatrixLocation = glGetUniformLocation(ProgramId, "ProjMatrix");
	ViewMatrixLocation = glGetUniformLocation(ProgramId, "ViewMatrix");
	WorldMatrixLocation = glGetUniformLocation(ProgramId, "WorldMatrix");

	// render loop
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		double currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		processInput(window);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 model = glm::mat4(1.0);

		shaderFloor.Use();
		glm::mat4 projection = pCamera->GetProjectionMatrix();
		glm::mat4 view = pCamera->GetViewMatrix();
		shaderFloor.SetMat4("projection", projection);
		shaderFloor.SetMat4("view", view);

		// Draw skybox
		glm::vec3 cubePositions[] = {
			glm::vec3(0.0f, 0.0f, 0.0f),
		};

		for (unsigned int i = 0; i < sizeof(cubePositions) / sizeof(cubePositions[0]); i++) {
			// calculate the model matrix for each object and pass it to shader before drawing
			glm::mat4 worldTransf = glm::translate(glm::mat4(1.0), cubePositions[i]);
			//glUniformMatrix4fv(WorldMatrixLocation, 1, GL_FALSE, glm::value_ptr(worldTransf));

			RenderSkybox(skyboxVBO, skyboxEBO);
		}

		// Draw floor
		glBindVertexArray(floorVAO);
		glBindTexture(GL_TEXTURE_2D, floorTexture);
		model = glm::mat4();
		shaderFloor.SetMat4("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		shaderBlending.Use();
		shaderBlending.SetMat4("projection", projection);
		shaderBlending.SetMat4("view", view);

		// Draw vegetation
		glBindVertexArray(grassVAO);
		glBindTexture(GL_TEXTURE_2D, grassTexture);
		model = glm::mat4();
		shaderFloor.SetMat4("model", model);

		drawGrass(shaderBlending, model, glm::vec3(10.0f, 20.0f, 40.0f));
		drawGrass(shaderBlending, model, glm::vec3(30.0f, -20.0f, 30.0f));

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	Cleanup();

	// Clear floor VAO

	// Clear grass VAO

	// glfw: terminate, clearing all previously allocated GLFW resources
	glfwTerminate();
	return 0;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		pCamera->ProcessKeyboard(ECameraMovementType::FORWARD, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		pCamera->ProcessKeyboard(ECameraMovementType::BACKWARD, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		pCamera->ProcessKeyboard(ECameraMovementType::LEFT, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		pCamera->ProcessKeyboard(ECameraMovementType::RIGHT, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS)
		pCamera->ProcessKeyboard(ECameraMovementType::UP, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS)
		pCamera->ProcessKeyboard(ECameraMovementType::DOWN, (float)deltaTime);

	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
		int width, height;
		glfwGetWindowSize(window, &width, &height);
		pCamera->Reset(width, height);

	}
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	pCamera->Reshape(width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	pCamera->MouseControl((float)xpos, (float)ypos);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yOffset)
{
	pCamera->ProcessMouseScroll((float)yOffset);
}

unsigned int CreateTexture(const std::string& strTexturePath)
{
	unsigned int textureId = -1;

	// load image, create texture and generate mipmaps
	int width, height, nrChannels;
	//stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
	unsigned char* data = stbi_load(strTexturePath.c_str(), &width, &height, &nrChannels, 0);
	if (data) {
		GLenum format;
		if (nrChannels == 1)
			format = GL_RED;
		else if (nrChannels == 3)
			format = GL_RGB;
		else if (nrChannels == 4)
			format = GL_RGBA;

		glGenTextures(1, &textureId);
		glBindTexture(GL_TEXTURE_2D, textureId);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else {
		std::cout << "Failed to load texture: " << strTexturePath << std::endl;
	}
	stbi_image_free(data);

	return textureId;
}