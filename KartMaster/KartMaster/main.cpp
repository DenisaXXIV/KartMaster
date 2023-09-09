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
#include <ext/matrix_clip_space.hpp>
#include <gtc/type_ptr.hpp>

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

float g_fKa = 0.3;
float g_fKd = 0.5;
float g_fKs = 0.3;
float g_fN = 1;

// timing
double deltaTime = 0.0f; // time between current frame and last frame 
double lastFrame = 0.0f;

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

unsigned int CreateTexture(const std::string& strTexturePath);

void draw2D(Shader& shaderBlending, glm::mat4& model, const glm::vec3& position, const glm::vec3& scale)
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
		glm::mat4 rotatedModel = model;
		rotatedModel = glm::translate(rotatedModel, position);
		rotatedModel = glm::rotate(rotatedModel, angle, glm::vec3(0.0f, 1.0f, 0.0f));
		rotatedModel = glm::scale(rotatedModel, scale);

		shaderBlending.SetMat4("model", rotatedModel);
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

	// set up vertex data (and buffer(s)) and configure vertex attributes 
	float verticesLightCube[] = {
		// Positions
		-0.1f, -0.1f, -0.1f, // 0
		 0.1f, -0.1f, -0.1f, // 1
		 0.1f,  0.1f, -0.1f, // 2
		-0.1f,  0.1f, -0.1f, // 3
		-0.1f, -0.1f,  0.1f, // 4
		 0.1f, -0.1f,  0.1f, // 5
		 0.1f,  0.1f,  0.1f, // 6
		-0.1f,  0.1f,  0.1f  // 7
	};

	unsigned int indicesLightCube[] = {
		0, 1, 2, // Front face
		2, 3, 0,
		4, 5, 6, // Back face
		6, 7, 4,
		1, 5, 6, // Right face
		6, 2, 1,
		4, 0, 3, // Left face
		3, 7, 4,
		3, 2, 6, // Top face
		6, 7, 3,
		4, 5, 1, // Bottom face
		1, 0, 4
	};

	unsigned int lightCubeVBO, lightCubeVAO, lightCubeEBO;
	glGenVertexArrays(1, &lightCubeVAO);
	glGenBuffers(1, &lightCubeVBO);
	glGenBuffers(1, &lightCubeEBO);

	// Step 2: Bind VAO
	glBindVertexArray(lightCubeVAO);

	// Step 3: Bind VBO and EBO
	glBindBuffer(GL_ARRAY_BUFFER, lightCubeVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lightCubeEBO);

	// Step 4: Transfer data to GPU
	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesLightCube), verticesLightCube, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicesLightCube), indicesLightCube, GL_STATIC_DRAW);

	// Step 5: Configure Vertex Attributes
	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

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
	unsigned int grassTexture = CreateTexture(strSourcePath + "Textures\\grass3.png");

	// Tree texture
	unsigned int treeTexture = CreateTexture(strSourcePath + "Textures\\tree.png");

	// Tree texture
	unsigned int trafficLightTexture = CreateTexture(strSourcePath + "Textures\\semafor.png");

	// Tree texture
	unsigned int firTexture = CreateTexture(strSourcePath + "Textures\\brad.png");

	// Rock texture
	unsigned int rockTexture = CreateTexture(strSourcePath + "Textures\\rock.png");

	// Big stone texture
	unsigned int stoneTexture = CreateTexture(strSourcePath + "Textures\\stone.png");

	// Overlay texture
	unsigned int overlayTexture = CreateTexture(strSourcePath + "Textures\\kart.png");

	glm::vec3 lightPos(0.0f, 0.0f, 2.0f);

	// Create camera
	pCamera = new Camera(SCR_WIDTH, SCR_HEIGHT, glm::vec3(0.0, 0.0, 3.0));
	Shader shaderSkybox((strSourcePath + "Shaders\\skybox.vs").c_str(),
		(strSourcePath + "Shaders\\skybox.fs").c_str());
	Shader shaderFloor((strSourcePath + "Shaders\\Floor.vs").c_str(),
		(strSourcePath + "Shaders\\Floor.fs").c_str());
	Shader shaderBlending((strSourcePath + "Shaders\\Blending.vs").c_str(),
		(strSourcePath + "Shaders\\Blending.fs").c_str());
	Shader lightingShader((strSourcePath + "Shaders\\PhongLight.vs").c_str(),
		(strSourcePath + "Shaders\\PhongLight.fs").c_str());
	Shader lampShader((strSourcePath + "Shaders\\Lamp.vs").c_str(),
		(strSourcePath + "Shaders\\Lamp.fs").c_str());

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
		glm::vec3 dim = glm::vec3(0.1, 0.1, 0.1);

		draw2D(shaderBlending, model, glm::vec3(3, -0.45, 2.5), dim);
		draw2D(shaderBlending, model, glm::vec3(3.4, -0.45, 2.6), dim);
		draw2D(shaderBlending, model, glm::vec3(2.8, -0.45, 2.4), dim);
		draw2D(shaderBlending, model, glm::vec3(3.2, -0.45, 2.7), dim);
		draw2D(shaderBlending, model, glm::vec3(3, -0.45, 2.6), dim);
		draw2D(shaderBlending, model, glm::vec3(3.4, -0.45, 2.3), dim);
		draw2D(shaderBlending, model, glm::vec3(2.8, -0.45, 2.7), dim);
		draw2D(shaderBlending, model, glm::vec3(3.2, -0.45, 2.4), dim);
		draw2D(shaderBlending, model, glm::vec3(2, -0.45, 2.5), dim);
		draw2D(shaderBlending, model, glm::vec3(2.4, -0.45, 2.6), dim);
		draw2D(shaderBlending, model, glm::vec3(1.8, -0.45, 2.4), dim);
		draw2D(shaderBlending, model, glm::vec3(2.2, -0.45, 2.7), dim);
		draw2D(shaderBlending, model, glm::vec3(2, -0.45, 2.6), dim);
		draw2D(shaderBlending, model, glm::vec3(2.4, -0.45, 2.3), dim);
		draw2D(shaderBlending, model, glm::vec3(2.8, -0.45, 2.7), dim);
		draw2D(shaderBlending, model, glm::vec3(2.2, -0.45, 2.4), dim);
		draw2D(shaderBlending, model, glm::vec3(3, -0.45, 3.5), dim);
		draw2D(shaderBlending, model, glm::vec3(3.4, -0.45, 3.6), dim);
		draw2D(shaderBlending, model, glm::vec3(2.8, -0.45, 3.4), dim);
		draw2D(shaderBlending, model, glm::vec3(3.2, -0.45, 3.7), dim);
		draw2D(shaderBlending, model, glm::vec3(3, -0.45, 3.6), dim);
		draw2D(shaderBlending, model, glm::vec3(3.4, -0.45, 3.3), dim);
		draw2D(shaderBlending, model, glm::vec3(2.8, -0.45, 3.7), dim);
		draw2D(shaderBlending, model, glm::vec3(3.2, -0.45, 3.4), dim);
		draw2D(shaderBlending, model, glm::vec3(2, -0.45, 3.5), dim);
		draw2D(shaderBlending, model, glm::vec3(2.4, -0.45, 3.6), dim);
		draw2D(shaderBlending, model, glm::vec3(1.8, -0.45, 3.4), dim);
		draw2D(shaderBlending, model, glm::vec3(2.2, -0.45, 3.7), dim);
		draw2D(shaderBlending, model, glm::vec3(2, -0.45, 3.6), dim);
		draw2D(shaderBlending, model, glm::vec3(2.4, -0.45, 3.3), dim);
		draw2D(shaderBlending, model, glm::vec3(2.8, -0.45, 3.7), dim);
		draw2D(shaderBlending, model, glm::vec3(2.2, -0.45, 3.4), dim);
		draw2D(shaderBlending, model, glm::vec3(3, -0.45, 4), dim);
		draw2D(shaderBlending, model, glm::vec3(3.4, -0.45, 4.1), dim);
		draw2D(shaderBlending, model, glm::vec3(2.8, -0.45, 3.9), dim);
		draw2D(shaderBlending, model, glm::vec3(3.2, -0.45, 4.2), dim);
		draw2D(shaderBlending, model, glm::vec3(3, -0.45, 4.1), dim);
		draw2D(shaderBlending, model, glm::vec3(3.4, -0.45, 3.8), dim);
		draw2D(shaderBlending, model, glm::vec3(2.8, -0.45, 4.2), dim);
		draw2D(shaderBlending, model, glm::vec3(3.2, -0.45, 3.9), dim);
		draw2D(shaderBlending, model, glm::vec3(2, -0.45, 4), dim);
		draw2D(shaderBlending, model, glm::vec3(2.4, -0.45, 4.1), dim);
		draw2D(shaderBlending, model, glm::vec3(1.8, -0.45, 3.9), dim);
		draw2D(shaderBlending, model, glm::vec3(2.2, -0.45, 4.2), dim);
		draw2D(shaderBlending, model, glm::vec3(2, -0.45, 4.1), dim);
		draw2D(shaderBlending, model, glm::vec3(2.4, -0.45, 3.8), dim);
		draw2D(shaderBlending, model, glm::vec3(2.8, -0.45, 4.2), dim);
		draw2D(shaderBlending, model, glm::vec3(2.2, -0.45, 3.9), dim);
		draw2D(shaderBlending, model, glm::vec3(3, -0.45, 4.5), dim);
		draw2D(shaderBlending, model, glm::vec3(3.4, -0.45, 4.6), dim);
		draw2D(shaderBlending, model, glm::vec3(2.8, -0.45, 4.4), dim);
		draw2D(shaderBlending, model, glm::vec3(3.2, -0.45, 4.7), dim);
		draw2D(shaderBlending, model, glm::vec3(3, -0.45, 4.6), dim);
		draw2D(shaderBlending, model, glm::vec3(3.4, -0.45, 4.3), dim);
		draw2D(shaderBlending, model, glm::vec3(2.8, -0.45, 4.7), dim);
		draw2D(shaderBlending, model, glm::vec3(3.2, -0.45, 4.4), dim);
		draw2D(shaderBlending, model, glm::vec3(2, -0.45, 4.5), dim);
		draw2D(shaderBlending, model, glm::vec3(2.4, -0.45, 4.6), dim);
		draw2D(shaderBlending, model, glm::vec3(1.8, -0.45, 4.4), dim);
		draw2D(shaderBlending, model, glm::vec3(2.2, -0.45, 4.7), dim);
		draw2D(shaderBlending, model, glm::vec3(2, -0.45, 4.6), dim);
		draw2D(shaderBlending, model, glm::vec3(2.4, -0.45, 4.3), dim);
		draw2D(shaderBlending, model, glm::vec3(2.8, -0.45, 4.7), dim);
		draw2D(shaderBlending, model, glm::vec3(2.2, -0.45, 4.4), dim);
		draw2D(shaderBlending, model, glm::vec3(-1.7, -0.45, -2), dim);
		draw2D(shaderBlending, model, glm::vec3(-3, -0.45, -4), dim);
		draw2D(shaderBlending, model, glm::vec3(3.2, -0.45, -4.1), dim);
		draw2D(shaderBlending, model, glm::vec3(-2.2, -0.45, -3), dim);
		draw2D(shaderBlending, model, glm::vec3(2, -0.45, -3), dim);
		draw2D(shaderBlending, model, glm::vec3(-3.2, -0.45, -1.4), dim);
		draw2D(shaderBlending, model, glm::vec3(3, -0.45, -1.4), dim);
		draw2D(shaderBlending, model, glm::vec3(4, -0.45, 0), dim);
		draw2D(shaderBlending, model, glm::vec3(-0.3, -0.45, -2.8), dim);
		draw2D(shaderBlending, model, glm::vec3(-0.8, -0.45, -1), dim);
		draw2D(shaderBlending, model, glm::vec3(0.5, -0.45, -1.3), dim);


		// Draw trees
		glBindVertexArray(grassVAO);
		glBindTexture(GL_TEXTURE_2D, treeTexture);
		model = glm::mat4();
		shaderFloor.SetMat4("model", model);
		draw2D(shaderBlending, model, glm::vec3(3, 0.1, 4), glm::vec3(1, 1, 1));
		draw2D(shaderBlending, model, glm::vec3(-3, 0.1, 4), glm::vec3(1.5, 1.5, 1.5));
		draw2D(shaderBlending, model, glm::vec3(-3.5, 0.1, -3), glm::vec3(1.2, 1.2, 1.2));
		draw2D(shaderBlending, model, glm::vec3(1.8, 0.1, -3), glm::vec3(1.2, 1.2, 1.2));
		draw2D(shaderBlending, model, glm::vec3(3.3, 0.1, -1.8), glm::vec3(1.1, 1.1, 1.1));
		draw2D(shaderBlending, model, glm::vec3(1.4, 0.1, 1.2), glm::vec3(1.4, 1.4, 1.4));



		// Draw traffic light
		glBindVertexArray(grassVAO);
		glBindTexture(GL_TEXTURE_2D, trafficLightTexture);
		model = glm::mat4();
		shaderFloor.SetMat4("model", model);
		draw2D(shaderBlending, model, glm::vec3(-3, -0.3, 0), glm::vec3(0.15, 0.5, 0.15));
		draw2D(shaderBlending, model, glm::vec3(3.8, -0.3, -1), glm::vec3(0.15, 0.5, 0.15));
		draw2D(shaderBlending, model, glm::vec3(-0.8, -0.3, -1.9), glm::vec3(0.15, 0.5, 0.15));

		// Draw fir
		glBindVertexArray(grassVAO);
		glBindTexture(GL_TEXTURE_2D, firTexture);
		model = glm::mat4();
		shaderFloor.SetMat4("model", model);
		draw2D(shaderBlending, model, glm::vec3(-1, 0.1, -4), glm::vec3(1, 1.2, 1));
		draw2D(shaderBlending, model, glm::vec3(-4.5, 0.1, 2), glm::vec3(1, 1.2, 1));
		draw2D(shaderBlending, model, glm::vec3(-3.5, 0.1, -1), glm::vec3(1, 1.2, 1));
		draw2D(shaderBlending, model, glm::vec3(0.3, 0.1, 3.7), glm::vec3(1, 1.2, 1));
		draw2D(shaderBlending, model, glm::vec3(0.6, 0.1, -1.7), glm::vec3(0.7, 1.2, 0.7));

		// Draw stone
		glBindVertexArray(grassVAO);
		glBindTexture(GL_TEXTURE_2D, stoneTexture);
		model = glm::mat4();
		shaderFloor.SetMat4("model", model);
		draw2D(shaderBlending, model, glm::vec3(-4.5, -0.2, 4.5), glm::vec3(0.7, 0.8, 0.7));
		draw2D(shaderBlending, model, glm::vec3(4.5, -0.1, -4), glm::vec3(1, 1.2, 1));

		// Draw rock
		glBindVertexArray(grassVAO);
		glBindTexture(GL_TEXTURE_2D, rockTexture);
		model = glm::mat4();
		shaderFloor.SetMat4("model", model);
		draw2D(shaderBlending, model, glm::vec3(-2.1, -0.45, 0.4), glm::vec3(0.2, 0.2, 0.2));
		draw2D(shaderBlending, model, glm::vec3(-3.5, -0.45, 3.7), glm::vec3(0.2, 0.2, 0.2));


		float time = glfwGetTime();

		// Calculate the angle for the rotation (in radians)
		float rotationAngle = time * glm::radians(5.0f); // Adjust the rotation speed as needed

		// Calculate the new position of the light cube
		float radius = 6.0f; // Radius of the circular path
		float lightX = sin(rotationAngle) * radius; // Calculate x-coordinate
		float lightY = cos(rotationAngle) * radius; // Calculate z-coordinate
		float lightZ = 0.0f; // Height of the light cube

		// Create a new model matrix for the light cube
		glm::mat4 lightModel = glm::mat4(1.0f); // Initialize as identity matrix
		lightModel = glm::translate(lightModel, glm::vec3(lightX, lightY, lightZ));
		//lightModel = glm::rotate(lightModel, rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate around Y-axis


		// Use the light cube shader program
		lampShader.Use();

		// Set shader uniforms
		lampShader.SetMat4("model", lightModel);
		lampShader.SetMat4("view", view);
		lampShader.SetMat4("projection", projection);

		// Draw the light cube
		glBindVertexArray(lightCubeVAO);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);


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
	//if (glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS)
	//	pCamera->ProcessKeyboard(ECameraMovementType::UP, (float)deltaTime);
	//if (glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS)
	//	pCamera->ProcessKeyboard(ECameraMovementType::DOWN, (float)deltaTime);

	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
		int width, height;
		glfwGetWindowSize(window, &width, &height);
		pCamera->Reset(width, 0.5f);

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