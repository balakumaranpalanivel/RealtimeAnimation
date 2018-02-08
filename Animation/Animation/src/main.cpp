
//Some Windows Headers (For Time, IO, etc.)
#include <windows.h>
#include <mmsystem.h>

#include <GL/glew.h>

#include <GLFW/glfw3.h>
#include <iostream>

#include <string> 
#include <fstream>
#include <iostream>
#include <sstream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "CShader.h" 
#include "CModel.h"
#include "Camera.h"
#include "Object3D.h"

#include <stb_image.h>

// Macro for indexing vertex buffer
#define BUFFER_OFFSET(i) ((char *)NULL + (i))
const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 800;

using namespace std;

CShader modelShader_Reflection;
CShader modelLoader_Normal;
CShader modelLoader;
CShader simpleShader;
CShader skyBoxShader;

CModel nanosuitModel, nanosuitModel1, modelTopRotor, modelMachineGun,
		modelLeftTail, modelRightTail, modelBody;
Object3D nanosuitTransform;
Object3D nanosuitTransform1;

Object3D bodyTransform,
	topRotorTransform,
	tailLeftRotorTransform,
	tailRightRotorTransform;

glm::vec3 translateVector = glm::vec3(0.0f, -10.0f, 0.0f);
glm::vec3 scaleVector = glm::vec3(0.5f, 0.5f, 0.5f);
glm::vec3 eulerAngles;

glm::mat4 model;
glm::mat4 orthoProjection;
glm::mat4 orthoView;
bool keys[1024];
bool firstMouse = true;

static double last_time = 0;
float radius = 10.0f;
double curr_time;
double delta;
GLfloat lastX = SCR_WIDTH / 2.0;
GLfloat lastY = SCR_HEIGHT / 2.0;

float rotate_y_top_rotor = 0.0f,
	rotate_z_left_rotor = 0.0f;
bool isFPS = false;
GLFWwindow* window;
const GLFWvidmode* videMode;
Camera newCamera(glm::vec3(0.0f, 0.0f, 0.0f));

glm::mat4 bodyRotate = glm::mat4();
glm::mat4 localBody = glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, 0.0f));
glm::mat4 projection;

float grid_vertex_count = 0;
float *grid_vertex_points;

GLuint element_buffer_length = 0;
GLuint *element_buffer;

unsigned int skyboxVAO, skyboxVBO;
unsigned int cubemapTexture;

void drawGrid(int nHalfSize)
{
	grid_vertex_count = pow((nHalfSize * 2) + 1, 2);
	grid_vertex_points = new float[grid_vertex_count * 3];
	int j = 0;
	for (float x = -nHalfSize; x <= nHalfSize; x++)
	{
		for (float y = -nHalfSize; y <= nHalfSize; y++)
		{
			grid_vertex_points[j++] = x;
			grid_vertex_points[j++] = 0;
			grid_vertex_points[j++] = y;
		}
	}

	j = 0;
	int vertices_in_row = ((nHalfSize * 2) + 1);
	element_buffer_length = ((2 * pow(vertices_in_row, 2)) - (2 * vertices_in_row)) * 2;
	element_buffer = new GLuint[element_buffer_length];
	for (int i = 0; i <= (grid_vertex_count - vertices_in_row); i += vertices_in_row)
	{
		for (int k = i; k <= (i + vertices_in_row) - 1; k++)
		{
			if (k != (i + vertices_in_row) - 1)
			{
				// 0 to 1
				element_buffer[j++] = k;
				element_buffer[j++] = k + 1;
			}

			if (k + vertices_in_row < grid_vertex_count)
			{
				// 0 to 3
				element_buffer[j++] = k;
				element_buffer[j++] = k + vertices_in_row;
			}
		}
	}
}

void generateObjectBufferTeapot() {
	GLuint vp_vbo = 0;

	GLuint loc1 = glGetAttribLocation(simpleShader.glnProgramID, "vertex_position");
	//loc2 = glGetAttribLocation(shaderProgramID, "vertex_normals");

	glGenBuffers(1, &vp_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
	//glBufferData (GL_ARRAY_BUFFER, 3 * teapot_vertex_count * sizeof (float), teapot_vertex_points, GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, 3 * grid_vertex_count * sizeof(float), grid_vertex_points, GL_STATIC_DRAW);

	//GLuint vn_vbo = 0;
	//glGenBuffers (1, &vn_vbo);
	//glBindBuffer (GL_ARRAY_BUFFER, vn_vbo);
	//glBufferData (GL_ARRAY_BUFFER, 3 * teapot_vertex_count * sizeof (float), teapot_normals, GL_STATIC_DRAW);

	//glGenVertexArrays(1, &teapot_vao);
	//glBindVertexArray(teapot_vao);

	glEnableVertexAttribArray(loc1);
	glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	// Element buffer object
	GLuint ebo;
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, element_buffer_length * sizeof(GLuint), element_buffer, GL_STATIC_DRAW);
	//glEnableVertexAttribArray (loc2);
	//glBindBuffer (GL_ARRAY_BUFFER, vn_vbo);
	//glVertexAttribPointer (loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
}

void generateObjectBufferSkybox();

glm::vec3 fwd = glm::vec3(0, 0, -1);
glm::vec4 rght = glm::vec4(1, 0, 0, 0);
glm::vec3 up = glm::vec3(0, 1, 0);

void display()
{

	// tell GL to only draw onto a pixel if the shape is closer to the viewer
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	generateObjectBufferSkybox();

	projection = glm::perspective(newCamera.GetZoom(), (GLfloat)SCR_WIDTH / (GLfloat)SCR_HEIGHT, 0.1f, 100.0f);

	glDepthMask(GL_FALSE);
	skyBoxShader.Use();
	skyBoxShader.SetMat4("view", glm::mat4(glm::mat3(newCamera.GetViewMatrix())));
	skyBoxShader.SetMat4("projection", projection);

	glBindVertexArray(skyboxVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
	glDepthMask(GL_TRUE);

	simpleShader.Use();
	simpleShader.SetMat4("proj", projection);
	simpleShader.SetMat4("view", newCamera.GetViewMatrix());
	simpleShader.SetMat4("model", glm::mat4());
	glDrawElements(GL_LINES, element_buffer_length, GL_UNSIGNED_INT, 0);

	modelLoader_Normal.Use();

	// Perspective projection viewport
	nanosuitTransform.rotateLocalQuat(0.0f, rotate_y_top_rotor, 0.0f);
	modelLoader_Normal.SetMat4("projection", projection);
	modelLoader_Normal.SetMat4("view", newCamera.GetViewMatrix());
	modelLoader_Normal.SetMat4("model", nanosuitTransform.getTransformMatrix());
	modelLoader_Normal.SetVec3("viewPos", newCamera.GetPosition());
	nanosuitModel.Draw(modelLoader_Normal);


	modelLoader.Use();

	// Perspective projection viewport
	nanosuitTransform1.rotateLocalQuat(0.0f, rotate_y_top_rotor, 0.0f);
	modelLoader.SetMat4("projection", projection);
	modelLoader.SetMat4("view", newCamera.GetViewMatrix());
	modelLoader.SetMat4("model", nanosuitTransform1.getTransformMatrix());
	modelLoader.SetVec3("viewPos", newCamera.GetPosition());
	nanosuitModel.Draw(modelLoader);

	//modelShader_Reflection.Use();
	//modelShader_Reflection.SetMat4("projection", projection);
	//modelShader_Reflection.SetMat4("view", newCamera.GetViewMatrix());
	//modelShader_Reflection.SetVec3("viewPos", newCamera.GetPosition());

	//glm::mat4 global1 = glm::mat4();

	//glm::mat4 globalBody = global1 * bodyTransform.getTransformMatrix();
	//modelShader_Reflection.SetMat4("model", globalBody);
	//modelShader_Reflection.SetVec3("aFragColor", glm::vec3(1.0f, 1.0f, 1.0f));
	//modelBody.Draw(modelShader_Reflection);

	//// Top Rotor
	//topRotorTransform.rotateLocalQuat(0.0f, rotate_y_top_rotor, 0.0f);
	//glm::mat4 globalTopRotor = globalBody * topRotorTransform.getTransformMatrix();
	//modelShader_Reflection.SetMat4("model", globalTopRotor);
	//modelShader_Reflection.SetVec3("aFragColor", glm::vec3(1.0f, 0.0f, 0.0f));
	//modelTopRotor.Draw(modelShader_Reflection);

	//// Tail Rotor Left
	//tailLeftRotorTransform.rotateLocalQuat(rotate_z_left_rotor, 0.0f, 0.0f);
	//glm::mat4 globalTailLeftRotor = globalBody * tailLeftRotorTransform.getTransformMatrix();
	//modelShader_Reflection.SetMat4("model", globalTailLeftRotor);
	//modelShader_Reflection.SetVec3("aFragColor", glm::vec3(0.0f, 0.0f, 1.0f));
	//modelLeftTail.Draw(modelShader_Reflection);

	//// Tail Rotor Right
	//tailRightRotorTransform.rotateLocalQuat(-rotate_z_left_rotor, 0.0f, 0.0f);
	//glm::mat4 globalTailRightRotor = globalBody * tailRightRotorTransform.getTransformMatrix();
	//modelShader_Reflection.SetMat4("model", globalTailRightRotor);
	//modelShader_Reflection.SetVec3("aFragColor", glm::vec3(1.0f, 1.0f, 0.0f));
	//modelRightTail.Draw(modelShader_Reflection);
}

void initScene()
{

	// Set up the shaders
	modelLoader_Normal.LoadShaders("../Animation/src/shaders/modelLoadingVertexShader_Normal.txt",
		"../Animation/src/shaders/modelLoadingFragmentShader_Normal.txt");

	modelLoader.LoadShaders("../Animation/src/shaders/modelLoadingVertexShader.txt",
		"../Animation/src/shaders/modelLoadingFragmentShader.txt");

	//modelShader_Reflection.LoadShaders("../Animation/src/shaders/modelLoadingVertexShader_Reflection.txt",
	//	"../Animation/src/shaders/modelLoadingFragmentShader_Reflection.txt");

	//simpleShader.LoadShaders("../Animation/src/shaders/simpleVertexShader.txt",
	//	"../Animation/src/shaders/simpleFragmentShader.txt");

	// Skybox shaders
	skyBoxShader.LoadShaders("../Animation/src/shaders/skyboxVertexShader.txt",
		"../Animation/src/shaders/skyboxFragmentShader.txt");


	//// Load 3D Model from a seperate file
	//modelTopRotor.LoadModel("../Assets/Models/helicopter/helicopter_top_rotor_local.obj");
	//modelMachineGun.LoadModel("../Assets/Models/helicopter/helicopter_machine_gun.obj");
	//modelLeftTail.LoadModel("../Assets/Models/helicopter/helicopter_left_tail_spin_local.obj");
	//modelRightTail.LoadModel("../Assets/Models/helicopter/helicopter_left_tail_spin_local.obj");
	//modelBody.LoadModel("../Assets/Models/helicopter/helicopter_body_local.obj");

	// Load the Nanosuit
	//nanosuitModel.LoadModel("../Assets/Models/BB8/bb8.obj");
	//nanosuitModel.LoadModel("../Assets/Models/ball/ball.obj");
	nanosuitModel.LoadModel("../Assets/Models/nanosuit/nanosuit.obj");

	// translate it down so it's at the center of the scene
	//model = glm::translate(model, translateVector);

	// scale the model to fit the viewports
	//model = glm::scale(model, scaleVector);

	//generateObjectBufferTeapot();

	topRotorTransform.translateLocal(glm::vec3(0.0f, 0.8f, -0.9f));
	tailLeftRotorTransform.translateLocal(glm::vec3(-0.05f, -0.25f, 4.5f));
	tailRightRotorTransform.translateLocal(glm::vec3(0.05f, -0.25f, 4.5f));

	nanosuitTransform.translateLocal(glm::vec3(0.0f, -5.0f, -5.0f));
	nanosuitTransform.scaleLocal(glm::vec3(0.5f, 0.5f, 0.5f));

	nanosuitTransform1.translateLocal(glm::vec3(5.0f, -5.0f, -5.0f));
	nanosuitTransform1.scaleLocal(glm::vec3(0.5f, 0.5f, 0.5f));
}

// Deltatime
GLfloat deltaTime = 0.0f;	// Time between current frame and last frame
GLfloat lastFrame = 0.0f;  	// Time of last frame

// Is called whenever a key is pressed/released via GLFW
void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
	if (GLFW_KEY_ESCAPE == key && GLFW_PRESS == action)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
		{
			keys[key] = true;
		}
		else if (action == GLFW_RELEASE)
		{
			keys[key] = false;
		}
	}
}


void MouseCallback(GLFWwindow *window, double xPos, double yPos)
{
	if (firstMouse)
	{
		lastX = xPos;
		lastY = yPos;
		firstMouse = false;
	}

	GLfloat xOffset = xPos - lastX;
	GLfloat yOffset = lastY - yPos;  // Reversed since y-coordinates go from bottom to left

	lastX = xPos;
	lastY = yPos;

	newCamera.ProcessMouseMovement(xOffset, yOffset, true);
}

void DoMovement()
{
	// Camera controls
	if (keys[GLFW_KEY_W])
	{
		newCamera.ProcessKeyboard(FORWARD, deltaTime);
	}

	if (keys[GLFW_KEY_S])
	{
		newCamera.ProcessKeyboard(BACKWARD, deltaTime);
	}

	if (keys[GLFW_KEY_A])
	{
		newCamera.ProcessKeyboard(LEFT, deltaTime);
	}

	if (keys[GLFW_KEY_D])
	{
		newCamera.ProcessKeyboard(RIGHT, deltaTime);
	}
}

void init()
{

	if (glfwInit() != GL_TRUE)
	{
		std::cout << "Failed to Initialise\n";
		return;
	}

	glfwDefaultWindowHints();
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "ThreeDEditor", NULL, NULL);

	videMode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	glfwSetWindowPos(window, (videMode->width - SCR_WIDTH)/2, (videMode->height - SCR_HEIGHT)/2);
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return;
	}

	glfwSetKeyCallback(window, KeyCallback);
	glfwSetCursorPosCallback(window, MouseCallback);

	glfwSwapInterval(1);
	glfwShowWindow(window);

	initScene();

}

void ProcessInputs()
{
	float deltaAngle = 0.01f;
	glm::quat MyQuaternion;

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		bodyTransform.ToggleRotation();
	}

	// move front
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		bodyTransform.translateLocal(
			glm::vec3(0.0f, 0.0f, -0.2f));
		if (isFPS)
		{
			newCamera.UpdateFPSPosition(FORWARD, 0.17);
		}
	}

	// move back
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		bodyTransform.translateLocal(glm::vec3(0.0f, 0.0f, 0.2f));
		if (isFPS)
		{
			
		}
	}

	// Pitch
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
	{
		bodyTransform.rotateLocalSpecial(deltaAngle, 0.0f, 0.0f);

		if (isFPS)
		{
			newCamera.UpdateFPSOrientation(deltaAngle + 0.25, 0.0f);
		}
	}

	if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
	{
		bodyTransform.rotateLocalSpecial(-deltaAngle, 0.0f, 0.0f);

		if (isFPS)
		{
			newCamera.UpdateFPSOrientation(-deltaAngle - 0.25, 0.0f);
		}
	}

	// yaw
	if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS)
	{
		bodyTransform.rotateLocalSpecial(0.0f, deltaAngle, 0.0f);
		if (isFPS)
		{
			newCamera.UpdateFPSOrientation(0.0f, -deltaAngle - 0.25);
		}
	}

	if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
	{
		bodyTransform.rotateLocalSpecial(0.0f, -deltaAngle, 0.0f);
		if (isFPS)
		{
			newCamera.UpdateFPSOrientation(0.0f, +deltaAngle + 0.25);
		}
	}

	// roll
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
	{
		bodyTransform.rotateLocalSpecial(0.0f, 0.0f, deltaAngle);
	}

	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
	{
		bodyTransform.rotateLocalSpecial(0.0f, 0.0f, -deltaAngle);
	}
}


void loop()
{
	while (glfwWindowShouldClose(window) == GL_FALSE)
	{
		glfwPollEvents();

		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
				
		ProcessInputs();
		DoMovement();

		curr_time = timeGetTime();
		delta = (curr_time - last_time);
		if (delta > 16.0f)
		{
			rotate_y_top_rotor = 0.01f;
			rotate_z_left_rotor = 0.1f;

			glm::vec3 eulerAngles(0.01f, 0.01f, 0.01f);
			glm::quat MyQuaternion = glm::quat(eulerAngles);

			last_time = curr_time;
		}
		display();
		glfwSwapBuffers(window);
	}
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const * path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

// loads a cubemap texture from 6 individual texture faces
// order:
// +X (right)
// -X (left)
// +Y (top)
// -Y (bottom)
// +Z (front) 
// -Z (back)
// -------------------------------------------------------
unsigned int loadCubemap(vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrComponents;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrComponents, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

void generateObjectBufferSkybox() {
	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
	float cubeVertices[] = {
		// positions          // normals
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
		0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
	};
	float skyboxVertices[] = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f,  1.0f
	};

	// cube VAO
	unsigned int cubeVAO, cubeVBO;
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);
	glBindVertexArray(cubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

	// skybox VAO
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
}

int main(int argc, char** argv){
	
	//call it like this
	drawGrid(10);

	init();

	// load textures
	// -------------
	vector<std::string> faces
	{
		"../Animation/src/skybox/right.jpg",
		"../Animation/src/skybox/left.jpg",
		"../Animation/src/skybox/top.jpg",
		"../Animation/src/skybox/bottom.jpg",
		"../Animation/src/skybox/front.jpg",
		"../Animation/src/skybox/back.jpg",
	};
	cubemapTexture = loadCubemap(faces);

	loop();

    return 0;
}











