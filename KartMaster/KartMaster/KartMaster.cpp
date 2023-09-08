//#include <stdlib.h> // necesare pentru citirea shader-elor
//#include <stdio.h>
//#include <math.h>
//
//#include <GL/glew.h>
//
//#include <GLM.hpp>
//#include <gtc/matrix_transform.hpp>
//#include <gtc/type_ptr.hpp>
//
//#include <glfw3.h>
//
//#include <iostream>
//#include <vector>
//
//#define STB_IMAGE_IMPLEMENTATION
//#include <stb_image.h>
//
//#include "Camera.h"
//#include "FileSystem.h"
//#include "Shader.h"
//
//#pragma comment (lib, "glfw3.lib")
//#pragma comment (lib, "glew32.lib")
//#pragma comment (lib, "OpenGL32.lib")
//
//// settings
//const unsigned int SCR_WIDTH = 1000;
//const unsigned int SCR_HEIGHT = 800;
//
//GLuint skyboxTexture;
//GLuint skyboxVAO, skyboxVBO, skyboxEBO;
//GLuint racetrackVAO, racetrackVBO, racetrackEBO;
//unsigned int VertexShaderId, FragmentShaderId, ProgramId, racetrackTextureLocation;
//GLuint ProjMatrixLocation, ViewMatrixLocation, WorldMatrixLocation;
//float g_fMixValue = 0.5;
//
//Camera* pCamera = nullptr;
//
//void Cleanup()
//{
//	delete pCamera;
//}
//
//
//// Shader-ul de varfuri / Vertex shader (este privit ca un sir de caractere)
//const GLchar* VertexShader =
//{
//   "#version 330\n"\
//   "layout (location = 0) in vec3 aPos;\n"\
//   "layout (location = 1) in vec2 aTexCoords;\n"\
//   "out vec2 TexCoord;\n"\
//   "uniform mat4 model;\n"\
//   "uniform mat4 view;\n"\
//   "uniform mat4 projection;\n"\
//   "void main()\n"\
//   "{\n"\
//	"TexCoord = aTexCoords;\n"\
//   "gl_Position = projection * view * model * vec4(aPos, 1.0);\n"\
//   "}\n"
//};
//// Shader-ul de fragment / Fragment shader (este privit ca un sir de caractere)
//const GLchar* FragmentShader =
//{
//   "#version 330\n"\
//   "out vec4 FragColor;\n"\
//
//   "in vec2 TexCoords;\n"\
//
//   "uniform sampler2D texture1;\n"\
//
//   "void main()\n"\
//   "{\n"\
//   "vec4 texColor = texture(texture1,TexCoords);\n"\
//   "if (texColor.a < 0.1 )\n"\
//   "discard;\n"\
//   "  FragColor = texColor;\n"\
//   "}\n"
//};
//
//GLuint loadCubemap(std::vector<std::string> faces)
//{
//	//initialize texture id and bind it
//	GLuint textureID;
//	glGenTextures(1, &textureID);
//	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
//
//	int width, height, nrChannels;
//	for (int i = 0; i < faces.size(); i++)
//	{
//		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
//		if (data)
//		{
//			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
//			//stbi_image_free(data);
//		}
//		else {
//			std::cout << "Could not load texture at path: " << faces[i] << std::endl;
//			//stbi_image_free(data);
//		}
//	}
//
//	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
//
//	return textureID;
//}
//
//float skyboxVertices[] = {
//	0.0f, 0.0f, 300.0f,   300.0f, 0.0f, 0.0f,   1.0f, 1.0f,
//	300.0f, 0.0f, 300.0f,   0.0f, 300.0f, 0.0f,   1.0f, 0.0f,
//	300.0f, 300.0f, 300.0f,   0.0f, 0.0f, 300.0f,   0.0f, 0.0f,
//	0.0f, 300.0f, 300.0f,   300.0f, 0.0f, 0.0f,   0.0f, 1.0f,
//	0.0f, 0.0f, 0.0f,   0.0f, 300.0f, 0.0f,   0.0f, 0.0f,
//	300.0f, 0.0f, 0.0f,   0.0f, 0.0f, 300.0f,   0.0f, 1.0f,
//	300.0f, 300.0f, 0.0f,   300.0f, 0.0f, 0.0f,   1.0f, 1.0f,
//	0.0f, 300.0f, 0.0f,   0.0f, 300.0f, 0.0f,   1.0f, 0.0f
//};
//
//GLuint skyboxIndices[] = {
//	0,1,2,
//	0,2,3,
//	1,5,6,
//	1,6,2,
//	5,4,7,
//	5,7,6,
//	4,0,3,
//	4,3,7,
//	0,5,1,
//	0,4,5,
//	3,2,6,
//	3,6,7
//};
//
//
//void CreateRacetrackVBO()
//{
//	float racetrackVertices[] = {
//		0.0f, 200.0f, 0.0f, 1.0f,
//		200.0f, 200.0f, 0.0f, 1.0f,
//		200.0f, 0.0f, 0.0f, 1.0f,
//		0.0f, 0.0f, 0.0f, 1.0f
//	};
//
//	float racetrackIndices[] = {
//		0,1,2,
//		0,2,3
//	};
//
//	glGenVertexArrays(1, &racetrackVAO);
//	glGenBuffers(1, &racetrackVBO);
//	glGenBuffers(1, &racetrackEBO);
//
//	glBindVertexArray(racetrackVAO);
//
//	glBindBuffer(GL_ARRAY_BUFFER, racetrackVBO);
//	glBufferData(GL_ARRAY_BUFFER, sizeof(racetrackVertices), racetrackVertices, GL_STATIC_DRAW);
//
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, racetrackEBO);
//	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(racetrackIndices), racetrackIndices, GL_STATIC_DRAW);
//
//	// position attribute
//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
//	glEnableVertexAttribArray(0);
//	// texture coord attribute
//	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
//	glEnableVertexAttribArray(1);
//}
//
//void CreateRacetrackTexture(const std::string& strTexturePath)
//{
//	glGenTextures(1, &racetrackTextureLocation);
//	glBindTexture(GL_TEXTURE_2D, racetrackTextureLocation);
//	// set the texture wrapping parameters
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//	// set texture filtering parameters
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//	// load image, create texture and generate mipmaps
//	int width, height, nrChannels;
//	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
//	unsigned char* data = stbi_load((strTexturePath + "Textures\\track.jpg").c_str(), &width, &height, &nrChannels, 0);
//	if (data) {
//		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
//		glGenerateMipmap(GL_TEXTURE_2D);
//	}
//	else {
//		std::cout << "Failed to load texture" << std::endl;
//	}
//	stbi_image_free(data);
//}
//
//const std::vector<std::string> skyboxFacesDay{
//	FileSystem::getPath("Textures\\skybox\\day\\right.png"),
//		FileSystem::getPath("Textures\\skybox\\day\\left.png"),
//		FileSystem::getPath("Textures\\skybox\\day\\top.png"),
//		FileSystem::getPath("Textures\\skybox\\day\\bottom.png"),
//		FileSystem::getPath("Textures\\skybox\\day\\front.png"),
//		FileSystem::getPath("Textures\\skybox\\day\\back.png")
//};
//const std::vector<std::string> skyboxFacesNight{
//	FileSystem::getPath("Textures\\skybox\\night\\right.png"),
//		FileSystem::getPath("Textures\\skybox\\night\\left.png"),
//		FileSystem::getPath("Textures\\skybox\\night\\top.png"),
//		FileSystem::getPath("Textures\\skybox\\night\\bottom.png"),
//		FileSystem::getPath("Textures\\skybox\\night\\front.png"),
//		FileSystem::getPath("Textures\\skybox\\night\\back.png")
//};
//
//void SkyboxInitDay()
//{
//	//skybox racetrachVAO
//	glGenVertexArrays(1, &skyboxVAO);
//	glGenBuffers(1, &skyboxVBO);
//	glBindVertexArray(skyboxVAO);
//	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
//	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
//	glEnableVertexAttribArray(0);
//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
//
//	//load textures
//	skyboxTexture = loadCubemap(skyboxFacesDay);
//}
//
//void SkyboxInitNight()
//{
//	//skybox racetrachVAO
//	glGenVertexArrays(1, &skyboxVAO);
//	glGenBuffers(1, &skyboxVBO);
//	glBindVertexArray(skyboxVAO);
//	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
//	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
//	glEnableVertexAttribArray(0);
//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
//
//	//load textures
//	skyboxTexture = loadCubemap(skyboxFacesNight);
//}
//
//void RenderSkybox()
//{
//	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxEBO);
//
//	int indexArraySize;
//	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &indexArraySize);
//	glDrawElements(GL_TRIANGLES, indexArraySize / sizeof(unsigned int), GL_UNSIGNED_INT, 0);
//}
//
//void DestroyskyboxVBO()
//{
//	glDeleteVertexArrays(1, &skyboxVAO);
//	glDeleteBuffers(1, &skyboxVBO);
//	glDeleteBuffers(1, &skyboxEBO);
//}
//
//void CreateShaders()
//{
//	VertexShaderId = glCreateShader(GL_VERTEX_SHADER);
//	glShaderSource(VertexShaderId, 1, &VertexShader, nullptr);
//	glCompileShader(VertexShaderId);
//
//	FragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
//	glShaderSource(FragmentShaderId, 1, &FragmentShader, nullptr);
//	glCompileShader(FragmentShaderId);
//
//	ProgramId = glCreateProgram();
//	glAttachShader(ProgramId, VertexShaderId);
//	glAttachShader(ProgramId, FragmentShaderId);
//	glLinkProgram(ProgramId);
//
//	GLint Success = 0;
//	GLchar ErrorLog[1024] = { 0 };
//
//	glGetProgramiv(ProgramId, GL_LINK_STATUS, &Success);
//	if (Success == 0) {
//		glGetProgramInfoLog(ProgramId, sizeof(ErrorLog), NULL, ErrorLog);
//		fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
//		exit(1);
//	}
//
//	glValidateProgram(ProgramId);
//	glGetProgramiv(ProgramId, GL_VALIDATE_STATUS, &Success);
//	if (!Success) {
//		glGetProgramInfoLog(ProgramId, sizeof(ErrorLog), NULL, ErrorLog);
//		fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
//		exit(1);
//	}
//
//	glUseProgram(ProgramId);
//
//	ProjMatrixLocation = glGetUniformLocation(ProgramId, "ProjMatrix");
//	ViewMatrixLocation = glGetUniformLocation(ProgramId, "ViewMatrix");
//	WorldMatrixLocation = glGetUniformLocation(ProgramId, "WorldMatrix");
//}
//
//void DestroyShaders()
//{
//	glUseProgram(0);
//
//	glDetachShader(ProgramId, VertexShaderId);
//	glDetachShader(ProgramId, FragmentShaderId);
//
//	glDeleteShader(FragmentShaderId);
//	glDeleteShader(VertexShaderId);
//
//	glDeleteProgram(ProgramId);
//}
//
//void Initialize(const std::string& strTexturePath)
//{
//	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // culoarea de fond a ecranului
//	//glEnable(GL_CULL_FACE);
//	glEnable(GL_DEPTH_TEST);
//	glEnable(GL_COLOR_MATERIAL);
//	glDisable(GL_LIGHTING);
//
//	//glFrontFace(GL_CCW);
//	//glCullFace(GL_BACK);
//
//	CreateRacetrackVBO();
//	CreateRacetrackTexture(strTexturePath);
//
//	SkyboxInitDay();
//	CreateShaders();
//
//	// Create camera
//	pCamera = new Camera(SCR_WIDTH, SCR_HEIGHT, glm::vec3(0.5, 0.5, 10));
//}
//
//void RenderFunction()
//{
//	glm::vec3 cubemapPosition[] = {
//	   glm::vec3(0.0f, 0.0f, 0.0f),
//	};
//
//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//	glUseProgram(ProgramId);
//
//	glm::mat4 projection = pCamera->GetProjectionMatrix();
//	glUniformMatrix4fv(ProjMatrixLocation, 1, GL_FALSE, glm::value_ptr(projection));
//
//	glm::mat4 view = pCamera->GetViewMatrix();
//	glUniformMatrix4fv(ViewMatrixLocation, 1, GL_FALSE, glm::value_ptr(view));
//
//	glBindVertexArray(skyboxVAO);
//
//	for (unsigned int i = 0; i < sizeof(cubemapPosition) / sizeof(cubemapPosition[0]); i++) {
//		// calculate the model matrix for each object and pass it to shader before drawing
//		glm::mat4 worldTransf = glm::translate(glm::mat4(1.0), cubemapPosition[i]);
//		glUniformMatrix4fv(WorldMatrixLocation, 1, GL_FALSE, glm::value_ptr(worldTransf));
//
//		RenderSkybox();
//	}
//}
//
//void Cleanup()
//{
//	DestroyShaders();
//	DestroyskyboxVBO();
//
//	delete pCamera;
//}
//
//void framebuffer_size_callback(GLFWwindow* window, int width, int height);
//void mouse_callback(GLFWwindow* window, double xpos, double ypos);
//void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
//void processInput(GLFWwindow* window);
//
//// timing
//double deltaTime = 0.0f;    // time between current frame and last frame
//double lastFrame = 0.0f;
//
//int main(int argc, char** argv)
//{
//	std::string strFullExeFileName = argv[0];
//	std::string strTexturePath;
//	const size_t last_slash_idx = strFullExeFileName.rfind('\\');
//	if (std::string::npos != last_slash_idx) {
//		strTexturePath = strFullExeFileName.substr(0, last_slash_idx);
//		strTexturePath = strTexturePath.substr(0, strTexturePath.find("\\x64"));
//		strTexturePath.append("\\KartMaster\\");
//	}
//
//	// glfw: initialize and configure
//	glfwInit();
//	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
//
//	// glfw window creation
//	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "KartMaster", NULL, NULL);
//	if (window == NULL) {
//		std::cout << "Failed to create GLFW window" << std::endl;
//		glfwTerminate();
//		return -1;
//	}
//
//	glfwMakeContextCurrent(window);
//	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
//	glfwSetCursorPosCallback(window, mouse_callback);
//	glfwSetScrollCallback(window, scroll_callback);
//
//	// tell GLFW to capture our mouse
//	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
//
//	glewInit();
//	Initialize(strTexturePath);
//
//	// TO DO
//	//Shader skyboxShader((strTexturePath + "Shaders\\skybox.vs").c_str(),
//	//	(strTexturePath + "Shaders\\skybox.fs").c_str());
//	//skyboxShader.Use();
//	//skyboxShader.SetInt("skybox", 0);
//
//	//Shader raceTrackShader((strTexturePath + "Shaders\\racetrack.vs").c_str(),
//	//	(strTexturePath + "Shaders\\racetrack.fs").c_str());
//	//Shader raceTrackShaderNight("", "");
//
//	// render loop
//	while (!glfwWindowShouldClose(window)) {
//		// per-frame time logic
//		double currentFrame = glfwGetTime();
//		deltaTime = currentFrame - lastFrame;
//		lastFrame = currentFrame;
//
//		// input
//		processInput(window);
//
//		// render
//		RenderFunction();
//
//		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
//		glfwSwapBuffers(window);
//		glfwPollEvents();
//	}
//
//	Cleanup();
//
//	// glfw: terminate, clearing all previously allocated GLFW resources
//	glfwTerminate();
//	return 0;
//}
//
//// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
//void processInput(GLFWwindow* window)
//{
//	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
//		glfwSetWindowShouldClose(window, true);
//
//	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
//		pCamera->ProcessKeyboard(ECameraMovementType::FORWARD, (float)deltaTime);
//	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
//		pCamera->ProcessKeyboard(ECameraMovementType::BACKWARD, (float)deltaTime);
//	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
//		pCamera->ProcessKeyboard(ECameraMovementType::LEFT, (float)deltaTime);
//	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
//		pCamera->ProcessKeyboard(ECameraMovementType::RIGHT, (float)deltaTime);
//	if (glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS)
//		pCamera->ProcessKeyboard(ECameraMovementType::UP, (float)deltaTime);
//	if (glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS)
//		pCamera->ProcessKeyboard(ECameraMovementType::DOWN, (float)deltaTime);
//
//	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
//		int width, height;
//		glfwGetWindowSize(window, &width, &height);
//		pCamera->Reset(width, height);
//
//	}
//}
//
//// glfw: whenever the window size changed (by OS or user resize) this callback function executes
//// ---------------------------------------------------------------------------------------------
//void framebuffer_size_callback(GLFWwindow* window, int width, int height)
//{
//	// make sure the viewport matches the new window dimensions; note that width and
//	// height will be significantly larger than specified on retina displays.
//	pCamera->Reshape(width, height);
//}
//
//void mouse_callback(GLFWwindow* window, double xpos, double ypos)
//{
//	pCamera->MouseControl((float)xpos, (float)ypos);
//}
//
//void scroll_callback(GLFWwindow* window, double xoffset, double yOffset)
//{
//	pCamera->ProcessMouseScroll((float)yOffset);
//}