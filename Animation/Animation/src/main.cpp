
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

// Macro for indexing vertex buffer
#define BUFFER_OFFSET(i) ((char *)NULL + (i))
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;

using namespace std;

CShader ourShader;
CModel ourModel, modelTopRotor, modelMachineGun,
		modelLeftTail, modelRightTail, modelBody;

int width = 800.0;
int height = 600.0;

// Camera
//glm::mat4 projection = glm::perspective<float>(45.0, ((float)(SCR_WIDTH) / (float)(SCR_HEIGHT)), 0.1f, 100.0f);
//
//glm::vec3 cameraPos = glm::vec3(0.0f, 1.5f, 5.0f);
//glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
//glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
//
//glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0.0f, 0.0f, 0.0f), cameraUp);

glm::vec3 translateVector = glm::vec3(0.0f, -10.0f, 0.0f);
glm::vec3 scaleVector = glm::vec3(0.5f, 0.5f, 0.5f);

glm::mat4 model;
glm::mat4 orthoProjection;
glm::mat4 orthoView;

static double  last_time = 0;
float radius = 10.0f;
double curr_time;
double delta;
bool firstMouse = true;

/*
	Declaring initiali values for the Euler angles
	Not defining "roll" angle for camera
*/
// Yaw is rotation of the object about the Y axis (look left/right)
float yaw = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.

// Pitch is rotation of the object about the x axis (look up/down)
float pitch = 0.0f;

// Shader Functions- click on + to expand
#pragma region SHADER_FUNCTIONS

bool LoadFile(const std::string& fileName, std::string& outShader)
{
	std::ifstream file(fileName);
	if (!file.is_open())
	{
		std::cout << "Error Loading file: " << fileName << " - impossible to open file" << std::endl;
		return false;
	}

	if (file.fail())
	{
		std::cout << "Error Loading file: " << fileName << std::endl;
		return false;
	}

	std::stringstream stream;
	stream << file.rdbuf();
	file.close();

	outShader = stream.str();

	return true;
}

void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
	// create a shader object
	GLuint ShaderObj = glCreateShader(ShaderType);

	if (ShaderObj == 0) {
		fprintf(stderr, "Error creating shader type %d\n", ShaderType);
		exit(0);
	}
	// Bind the source code to the shader, this happens before compilation
	glShaderSource(ShaderObj, 1, (const GLchar**)&pShaderText, NULL);
	// compile the shader and check for errors
	glCompileShader(ShaderObj);
	GLint success;
	// check for shader related errors using glGetShaderiv
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024];
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
		exit(1);
	}
	// Attach the compiled shader object to the program object
	glAttachShader(ShaderProgram, ShaderObj);
}

GLuint CompileShaders(const std::string& vsFilename, const std::string& psFilename)
{
	//Start the process of setting up our shaders by creating a program ID
	//Note: we will link all the shaders together into this ID
	GLuint shaderProgramID = glCreateProgram();
	if (shaderProgramID == 0) {
		fprintf(stderr, "Error creating shader program\n");
		exit(1);
	}

	// Create two shader objects, one for the vertex, and one for the fragment shader
	std::string vs, ps;
	LoadFile(vsFilename, vs);
	AddShader(shaderProgramID, vs.c_str(), GL_VERTEX_SHADER);
	LoadFile(psFilename, ps);
	AddShader(shaderProgramID, ps.c_str(), GL_FRAGMENT_SHADER);

	GLint Success = 0;
	GLchar ErrorLog[1024] = { 0 };

	// After compiling all shader objects and attaching them to the program, we can finally link it
	glLinkProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
		exit(1);
	}

	// program has been successfully linked but needs to be validated to check whether the program can execute given the current pipeline state
	glValidateProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgramID, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
		exit(1);
	}
	// Finally, use the linked shader program
	// Note: this program will stay in effect for all draw calls until you replace it with another or explicitly disable its use
	glUseProgram(shaderProgramID);
	return shaderProgramID;
}
#pragma endregion SHADER_FUNCTIONS

// VBO Functions - click on + to expand
#pragma region VBO_FUNCTIONS
GLuint VAO, VBO;

GLuint generateObjectBuffer(GLfloat vertices[], GLfloat colors[]) {
	GLuint numVertices = 3;
	// Genderate 1 generic buffer object, called VBO
	glGenBuffers(1, &VBO);
	// In OpenGL, we bind (make active) the handle to a target name and then execute commands on that target
	// Buffer will contain an array of vertices 
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// After binding, we now fill our object with data, everything in "Vertices" goes to the GPU
	glBufferData(GL_ARRAY_BUFFER, numVertices * 7 * sizeof(GLfloat), NULL, GL_STATIC_DRAW);
	// if you have more data besides vertices (e.g., vertex colours or normals), use glBufferSubData to tell the buffer when the vertices array ends and when the colors start
	glBufferSubData(GL_ARRAY_BUFFER, 0, numVertices * 3 * sizeof(GLfloat), vertices);
	glBufferSubData(GL_ARRAY_BUFFER, numVertices * 3 * sizeof(GLfloat), numVertices * 4 * sizeof(GLfloat), colors);

	// TODO for GLFW
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	
	return VBO;
}

void linkCurrentBuffertoShader(GLuint shaderProgramID) {
	GLuint numVertices = 3;
	// find the location of the variables that we will be using in the shader program
	GLuint positionID = glGetAttribLocation(shaderProgramID, "vPosition");
	GLuint colorID = glGetAttribLocation(shaderProgramID, "vColor");
	// Have to enable this
	glEnableVertexAttribArray(positionID);
	// Tell it where to find the position data in the currently active buffer (at index positionID)
	glVertexAttribPointer(positionID, 3, GL_FLOAT, GL_FALSE, 0, 0);
	// Similarly, for the color data.
	glEnableVertexAttribArray(colorID);
	glVertexAttribPointer(colorID, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(numVertices * 3 * sizeof(GLfloat)));
}

#pragma endregion VBO_FUNCTIONS

float rotate_y_top_rotor = 0.0f,
	rotate_z_left_rotor = 0.0f;

GLFWwindow* window;
const GLFWvidmode* videMode;
Camera primaryCamera;

void display()
{
	// tell GL to only draw onto a pixel if the shape is closer to the viewer
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	ourShader.Use();

	// Perspective projection viewport
	//glViewport(0, 0, SCR_WIDTH / 2, SCR_HEIGHT);
	primaryCamera.ComputeProjectViewFromInputs();
	ourShader.SetMat4("projection", primaryCamera.mProjection);
	ourShader.SetMat4("view", primaryCamera.mView);
	ourShader.SetVec3("viewPos", primaryCamera.mCameraPosition);

	glm::mat4 global1 = glm::mat4();

	// Body
	glm::mat4 localBody = glm::mat4();
	localBody = glm::scale(localBody, scaleVector);
	localBody = glm::rotate(localBody, rotate_y_top_rotor*0.1f, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 globalBody = global1 * localBody;
	ourShader.SetMat4("model", globalBody);
	ourShader.SetVec3("aFragColor", glm::vec3(1.0f, 1.0f, 1.0f));
	modelBody.Draw(ourShader);

	// Top Rotor
	glm::mat4 localTopRotor = glm::mat4();
	//localTopRotor = glm::scale(localTopRotor, scaleVector);
	localTopRotor = glm::rotate(localTopRotor, rotate_y_top_rotor, glm::vec3(0.0f, 1.0f, 0.0f));
	localTopRotor = glm::translate(localTopRotor, glm::vec3(0.0f, 0.10f, 0.0f));
	glm::mat4 globalTopRotor = globalBody * localTopRotor;
	ourShader.SetMat4("model", globalTopRotor);
	ourShader.SetVec3("aFragColor", glm::vec3(1.0f, 0.0f, 0.0f));
	modelTopRotor.Draw(ourShader);

	// Tail Rotor Left
	glm::mat4 localTailLeftRotor = glm::mat4();
	// translate
	localTailLeftRotor = glm::translate(localTailLeftRotor, glm::vec3(-0.05f, 2.87f, 5.5f));
	// rotation
	localTailLeftRotor = glm::rotate(localTailLeftRotor, rotate_z_left_rotor, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 globalTailLeftRotor = globalBody * localTailLeftRotor;
	ourShader.SetMat4("model", globalTailLeftRotor);
	ourShader.SetVec3("aFragColor", glm::vec3(0.0f, 0.0f, 1.0f));
	modelLeftTail.Draw(ourShader);

	// Tail Rotor Right
	glm::mat4 localTailRightRotor = glm::mat4();
	localTailRightRotor = glm::translate(localTailRightRotor, glm::vec3(0.05f, 2.87f, 5.5f));
	localTailRightRotor = glm::rotate(localTailRightRotor, -rotate_z_left_rotor, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 globalTailRightRotor = globalBody * localTailRightRotor;
	ourShader.SetMat4("model", globalTailRightRotor);
	ourShader.SetVec3("aFragColor", glm::vec3(1.0f, 1.0f, 0.0f));
	modelRightTail.Draw(ourShader);

	//ourShader.SetMat4("model", model);
	//ourShader.SetVec3("aFragColor", glm::vec3(1.0f, 1.0f, 1.0f));
	//ourModel.Draw(ourShader);

	//glm::mat4 model1 = glm::translate(model, glm::vec3(1.0f, 0.0f, 0.0f));
	//ourShader.SetMat4("model", model1);
	//ourShader.SetVec3("aFragColor", glm::vec3(1.0f, 0.0f, 0.0f));
	//modelTopRotor.Draw(ourShader);

	//glm::mat4 model2 = glm::translate(model, glm::vec3(1.0f, 0.0f, 0.0f));
	//ourShader.SetMat4("model", model2);
	//ourShader.SetVec3("aFragColor", glm::vec3(0.0f, 1.0f, 0.0f));
	//modelMachineGun.Draw(ourShader);

	//glm::mat4 model3 = glm::translate(model, glm::vec3(1.0f, 0.0f, 0.0f));
	//ourShader.SetMat4("model", model3);
	//ourShader.SetVec3("aFragColor", glm::vec3(0.0f, 0.0f, 1.0f));
	//modelLeftTail.Draw(ourShader);

	//glm::mat4 model4 = glm::translate(model, glm::vec3(1.0f, 0.0f, 0.0f));
	//ourShader.SetMat4("model", model4);
	//ourShader.SetVec3("aFragColor", glm::vec3(1.0f, 1.0f, 0.0f));
	//modelRightTail.Draw(ourShader);

	//glm::mat4 model5 = glm::translate(model, glm::vec3(1.0f, 0.0f, 0.0f));
	//ourShader.SetMat4("model", model5);
	//ourShader.SetVec3("aFragColor", glm::vec3(0.0f, 1.0f, 1.0f));
	//modelBody.Draw(ourShader);


	//// Orthographic projection viewport
	//glViewport(SCR_WIDTH / 2, 0, SCR_WIDTH / 2, SCR_HEIGHT);
	//orthoProjection = glm::ortho<float>(-2.0f, 2.0f, -2.0f, 2.0f, -200.0f, 200.0f);
	//ourShader.SetMat4("projection", orthoProjection);
	//orthoView = glm::lookAt(
	//	glm::vec3(1.0f, 1.5f, 1.5f),
	//	glm::vec3(0, 0, 0),
	//	cameraUp);
	//ourShader.SetMat4("view", orthoView);
	//ourShader.SetMat4("model", model);
	//ourShader.SetVec3("viewPos", cameraPos);
	//ourModel.Draw(ourShader);
}

void initScene()
{
	//// Create 3 vertices that make up a triangle that fits on the viewport 
	//GLfloat vertices[] = { -1.0f, -1.0f, 0.0f,
	//	1.0f, -1.0f, 0.0f,
	//	0.0f, 1.0f, 0.0f };
	//// Create a color array that identfies the colors of each vertex (format R, G, B, A)
	//GLfloat colors[] = { 0.0f, 1.0f, 0.0f, 1.0f,
	//	1.0f, 0.0f, 0.0f, 1.0f,
	//	0.0f, 0.0f, 1.0f, 1.0f };

	//// Set up the shaders
	//GLuint shaderProgramID = CompileShaders("../Animation/src/shaders/diffuse.vs",
	//	"../Animation/src/shaders/diffuse.ps");

	//// Put the vertices and colors into a vertex buffer object
	//generateObjectBuffer(vertices, colors);

	//// Link the current buffer to the shader
	//linkCurrentBuffertoShader(shaderProgramID);

	// Set up the shaders
	ourShader.LoadShaders("../Animation/src/shaders/modelLoadingVertexShader.txt",
		"../Animation/src/shaders/modelLoadingFragmentShader.txt");

	// Load 3D Model from a seperate file
	ourModel.LoadModel("../Assets/Models/helicopter/helicopter.obj");
	modelTopRotor.LoadModel("../Assets/Models/helicopter/helicopter_top_rotor.obj");
	modelMachineGun.LoadModel("../Assets/Models/helicopter/helicopter_machine_gun.obj");
	modelLeftTail.LoadModel("../Assets/Models/helicopter/helicopter_left_tail_spin_local.obj");
	modelRightTail.LoadModel("../Assets/Models/helicopter/helicopter_left_tail_spin_local.obj");
	modelBody.LoadModel("../Assets/Models/helicopter/helicopter_body.obj");

	// translate it down so it's at the center of the scene
	model = glm::translate(model, translateVector);

	// scale the model to fit the viewports
	model = glm::scale(model, scaleVector);

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

	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Raytracer Compute Shader", NULL, NULL);

	videMode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	glfwSetWindowPos(window, (videMode->width - SCR_WIDTH)/2, (videMode->height - SCR_HEIGHT)/2);
	glfwMakeContextCurrent(window);

	primaryCamera.mWindow = window;
	primaryCamera.SetMouseCallBack();

	// Initialize GLEW
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return;
	}

	glfwSwapInterval(1);
	glfwShowWindow(window);

	initScene();

}

void loop()
{
	while (glfwWindowShouldClose(window) == GL_FALSE)
	{
		glfwPollEvents();
		glViewport(0, 0, width, height);
		curr_time = timeGetTime();
		delta = (curr_time - last_time);
		if (delta > 16.0f)
		{
			rotate_y_top_rotor += 0.7f;
			rotate_z_left_rotor += 0.1f;

			// Update the view matrix to move the camera based on user input
			primaryCamera.mView = glm::lookAt(
				primaryCamera.mCameraPosition,
				primaryCamera.mCameraPosition + primaryCamera.mCameraFront,
				primaryCamera.mCameraUp);

			glm::vec3 eulerAngles(0.01f, 0.01f, 0.01f);
			glm::quat MyQuaternion = glm::quat(eulerAngles);

			// Constantly rotate the model about the Y axis
			//model = glm::rotate<float>(model, 0.01, glm::vec3(0.0f, 1.0f, 0.0f));
			//model = glm::toMat4(MyQuaternion) * model;
			last_time = curr_time;
		}
		display();
		glfwSwapBuffers(window);
	}
}

int main(int argc, char** argv){
	
	init();

	loop();

    return 0;
}











