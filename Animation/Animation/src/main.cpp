
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
const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 800;

using namespace std;

CShader ourShader;
CModel ourModel, modelTopRotor, modelMachineGun,
		modelLeftTail, modelRightTail, modelBody;

glm::vec3 translateVector = glm::vec3(0.0f, -10.0f, 0.0f);
glm::vec3 scaleVector = glm::vec3(0.5f, 0.5f, 0.5f);

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

GLFWwindow* window;
const GLFWvidmode* videMode;
Camera newCamera;

glm::mat4 bodyRotate = glm::mat4();
glm::mat4 localBody = glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, 0.0f));
glm::mat4 projection;

void display()
{
	// tell GL to only draw onto a pixel if the shape is closer to the viewer
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	ourShader.Use();

	projection = glm::perspective(newCamera.GetZoom(), (GLfloat)SCR_WIDTH / (GLfloat)SCR_HEIGHT, 0.1f, 100.0f);

	ourShader.SetMat4("projection", projection);
	ourShader.SetMat4("view", newCamera.GetViewMatrix());
	ourShader.SetVec3("viewPos", newCamera.GetPosition());

	glm::mat4 global1 = glm::mat4();

	//localBody = glm::mat4();
	//localBody = glm::scale(localBody, scaleVector);
	//localBody = glm::rotate(localBody, rotate_y_top_rotor*0.1f, glm::vec3(0.0f, 1.0f, 0.0f));
	//localBody = bodyRotate * localBody;
	// Body
	glm::mat4 globalBody = global1 * localBody;
	ourShader.SetMat4("model", globalBody);
	ourShader.SetVec3("aFragColor", glm::vec3(1.0f, 1.0f, 1.0f));
	modelBody.Draw(ourShader);

	// Top Rotor
	glm::mat4 localTopRotor = glm::mat4();
	//localTopRotor = glm::scale(localTopRotor, scaleVector);
	localTopRotor = glm::translate(localTopRotor, glm::vec3(0.0f, 0.8f, -0.9f));
	localTopRotor = glm::rotate(localTopRotor, rotate_y_top_rotor, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 globalTopRotor = globalBody * localTopRotor;
	ourShader.SetMat4("model", globalTopRotor);
	ourShader.SetVec3("aFragColor", glm::vec3(1.0f, 0.0f, 0.0f));
	modelTopRotor.Draw(ourShader);

	// Tail Rotor Left
	glm::mat4 localTailLeftRotor = glm::mat4();
	// translate
	localTailLeftRotor = glm::translate(localTailLeftRotor, glm::vec3(-0.05f, -0.25f, 4.5f));
	// rotation
	localTailLeftRotor = glm::rotate(localTailLeftRotor, rotate_z_left_rotor, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 globalTailLeftRotor = globalBody * localTailLeftRotor;
	ourShader.SetMat4("model", globalTailLeftRotor);
	ourShader.SetVec3("aFragColor", glm::vec3(0.0f, 0.0f, 1.0f));
	modelLeftTail.Draw(ourShader);

	// Tail Rotor Right
	glm::mat4 localTailRightRotor = glm::mat4();
	localTailRightRotor = glm::translate(localTailRightRotor, glm::vec3(0.05f, -0.25f, 4.5f));
	localTailRightRotor = glm::rotate(localTailRightRotor, -rotate_z_left_rotor, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 globalTailRightRotor = globalBody * localTailRightRotor;
	ourShader.SetMat4("model", globalTailRightRotor);
	ourShader.SetVec3("aFragColor", glm::vec3(1.0f, 1.0f, 0.0f));
	modelRightTail.Draw(ourShader);
}

void initScene()
{

	// Set up the shaders
	ourShader.LoadShaders("../Animation/src/shaders/modelLoadingVertexShader.txt",
		"../Animation/src/shaders/modelLoadingFragmentShader.txt");

	// Load 3D Model from a seperate file
	modelTopRotor.LoadModel("../Assets/Models/helicopter/helicopter_top_rotor_local.obj");
	modelMachineGun.LoadModel("../Assets/Models/helicopter/helicopter_machine_gun.obj");
	modelLeftTail.LoadModel("../Assets/Models/helicopter/helicopter_left_tail_spin_local.obj");
	modelRightTail.LoadModel("../Assets/Models/helicopter/helicopter_left_tail_spin_local.obj");
	modelBody.LoadModel("../Assets/Models/helicopter/helicopter_body_local.obj");

	// translate it down so it's at the center of the scene
	model = glm::translate(model, translateVector);

	// scale the model to fit the viewports
	model = glm::scale(model, scaleVector);

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

	newCamera.ProcessMouseMovement(xOffset, yOffset);
}

void DoMovement()
{
	// Camera controls
	if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP])
	{
		newCamera.ProcessKeyboard(FORWARD, deltaTime);
	}

	if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN])
	{
		newCamera.ProcessKeyboard(BACKWARD, deltaTime);
	}

	if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT])
	{
		newCamera.ProcessKeyboard(LEFT, deltaTime);
	}

	if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT])
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

	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Raytracer Compute Shader", NULL, NULL);

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
	glm::vec3 eulerAngles;
	glm::quat MyQuaternion;

	// Pitch
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
	{
		eulerAngles = glm::vec3(deltaAngle, 0.0f, 0.0f);
		MyQuaternion = glm::quat(eulerAngles);
		localBody = glm::toMat4(MyQuaternion) * localBody;
	}

	if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
	{
		eulerAngles = glm::vec3(-deltaAngle, 0.0f, 0.0f);
		MyQuaternion = glm::quat(eulerAngles);
		localBody = glm::toMat4(MyQuaternion) * localBody;
	}

	// yaw
	if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS)
	{
		eulerAngles = glm::vec3(0.0f, deltaAngle, 0.0f);
		MyQuaternion = glm::quat(eulerAngles);
		localBody = glm::toMat4(MyQuaternion) * localBody;
	}

	if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
	{
		eulerAngles = glm::vec3(0.0f, -deltaAngle, 0.0f);
		MyQuaternion = glm::quat(eulerAngles);
		localBody = glm::toMat4(MyQuaternion) * localBody;
	}

	// roll
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
	{
		eulerAngles = glm::vec3(0.0f, 0.0f, deltaAngle);
		MyQuaternion = glm::quat(eulerAngles);
		localBody = glm::toMat4(MyQuaternion) * localBody;
	}

	//if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	//{
	//	eulerAngles = glm::vec3(0.0f, 0.0f, -deltaAngle);
	//	MyQuaternion = glm::quat(eulerAngles);
	//	localBody = glm::toMat4(MyQuaternion) * localBody;
	//}
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
			rotate_y_top_rotor += 0.7f;
			rotate_z_left_rotor += 0.1f;

			glm::vec3 eulerAngles(0.01f, 0.01f, 0.01f);
			glm::quat MyQuaternion = glm::quat(eulerAngles);

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











