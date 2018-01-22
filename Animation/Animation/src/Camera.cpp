#include "Camera.h"

Camera::Camera()
{
	mHorizontalAngle = 3.14f;
	mVerticalAngle = 0.0f;
	mInitialFOV = 45.0f;
	mSpeed = 3.0f;
	mMouseSpeed = 0.005f;
	mWidth = 800;
	mHeight = 600;

	mCameraPosition = glm::vec3(0.0f, 1.5f, 10.0f);
	mCameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	mCameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

	mProjection = glm::perspective<float>(
		mInitialFOV,
		(float)mWidth / (float)mHeight,
		0.1f, 100.0f);
	mView = glm::lookAt(mCameraPosition, glm::vec3(0.0f, 0.0f, 0.0f), mCameraUp);

}

bool mButtonDown;

void Camera::SetMouseCallBack()
{
	glfwSetMouseButtonCallback(mWindow, mouse_callback);
}

static void mouse_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		if (GLFW_PRESS == action)
		{
			mButtonDown = true;
		}
		else if (GLFW_RELEASE == action)
		{
			mButtonDown = false;
		}
	}

	if (mButtonDown) {
		// do your drag here
		std::cout << "Dragged" << std::endl;
	}
}

void Camera::ComputeProjectViewFromInputs()
{
	double currentTime = glfwGetTime();
	float deltaTime = float(currentTime - mLastTime);
	mLastTime = currentTime;

	glfwGetCursorPos(mWindow, &xPos, &yPos);
	std::cout << "Xpos: " << xPos << " Ypos: " << yPos << std::endl;

	//if (xPos < 0)
	//{
	//	xPos = 0;
	//}
	//if (xPos > mWidth)
	//{
	//	xPos = mWidth;
	//}

	//if (yPos < 0)
	//{
	//	yPos = 0;
	//}
	//if (yPos > mHeight)
	//{
	//	yPos = mHeight;
	//}

	mHorizontalAngle += mMouseSpeed * deltaTime * float(mWidth / 2 - xPos);
	mVerticalAngle += mMouseSpeed * deltaTime * float(mHeight / 2 - xPos);
	std::cout << "Xpos: " << mHorizontalAngle << " Ypos: " <<
		mVerticalAngle << std::endl;

	//glm::vec3 direction(
	//	cos(mVerticalAngle) * sin(mHorizontalAngle),
	//	sin(mVerticalAngle),
	//	cos(mVerticalAngle) * cos(mHorizontalAngle)
	//);

	//glm::vec3 right = glm::vec3(
	//	sin(mHorizontalAngle - 3.14f / 2.0f),
	//	0,
	//	cos(mHorizontalAngle - 3.14f / 2.0f)
	//);

	//glm::vec3 up = glm::cross(right, direction);

	// Move forward
	if (glfwGetKey(mWindow, GLFW_KEY_UP) == GLFW_PRESS) {
		mCameraPosition += glm::vec3(0.0f, 0.0f, -1.0f) * deltaTime * mSpeed;
	}
	// Move backward
	if (glfwGetKey(mWindow, GLFW_KEY_DOWN) == GLFW_PRESS) {
		mCameraPosition -= glm::vec3(0.0f, 0.0f, -1.0f) * deltaTime * mSpeed;
	}
	// Strafe right
	if (glfwGetKey(mWindow, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		mCameraPosition += glm::vec3(1.0f, 0.0f, 0.0f) * deltaTime * mSpeed;
	}
	// Strafe left
	if (glfwGetKey(mWindow, GLFW_KEY_LEFT) == GLFW_PRESS) {
		mCameraPosition -= glm::vec3(1.0f, 0.0f, 0.0f) * deltaTime * mSpeed;
	}

	////float FoV = mInitialFOV - 5 * glfwGetMouseWheel();

	//// Projection matrix : 45&deg; Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	////mProjection = glm::perspective(glm::radians(mInitialFOV), mWidth / mHeight, 0.1f, 100.0f);

	//// Camera matrix
	//mView = glm::lookAt(
	//	mCameraPosition,           // Camera is here
	//	glm::vec3(0.0f, 0.0f, 0.0f), // and looks here : at the same position, plus "direction"
	//	up                  // Head is up (set to 0,-1,0 to look upside-down)
	//);

	//glfwSetCursorPos(mWindow, mWidth/2, mHeight/2);
}