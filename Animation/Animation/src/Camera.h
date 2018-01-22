#pragma once
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

class Camera
{
public:
	glm::vec3 mCameraPosition;
	glm::vec3 mCameraFront;
	glm::vec3 mCameraUp;

	glm::mat4 mProjection;
	glm::mat4 mView;

	float mHorizontalAngle;
	float mVerticalAngle;
	float mInitialFOV;
	float mSpeed;
	float mMouseSpeed;
	float mWidth;
	float mHeight;
	float mLastTime;
	double xPos = 0, yPos = 0;

	GLFWwindow* mWindow;

	Camera();
	void ComputeProjectViewFromInputs();
	bool mButtonDown;
	void SetMouseCallBack();

	friend static void mouse_callback(GLFWwindow* window, int button, int action, int mods);
};