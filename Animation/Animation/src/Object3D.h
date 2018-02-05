#pragma once

// Libraries
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>


// Class Object3D
// This class represent a generic 3D object on the scene.
class Object3D {

protected:

	// -------------------- Attributes -------------------- //

	// Transformation matrix that convert local-space
	// coordinates to world-space coordinates
	glm::mat4 mTransformMatrix;

public:

	// -------------------- Methods -------------------- //

	// Constructor
	Object3D() 
	{
		setToIdentity();
	}

	// Destructor
	virtual ~Object3D()
	{

	}

	// Return the transform matrix
	const glm::mat4& getTransformMatrix() const;

	// Set to the identity transform
	void setToIdentity();

	// Return the origin of object in world-space
	glm::vec3 getOrigin() const;

	// Translate the object in world-space
	void translateWorld(const glm::vec3& v);

	// Translate the object in local-space
	void translateLocal(const glm::vec3& v);

	// Rotate the object in world-space
	//void rotateWorld(const glm::vec3& axis, float angle);

	// Rotate the object in local-space
	void rotateLocal(const glm::vec3& axis, float angle);

	// Rotate the obejct in local-space using quat
	void rotateLocalQuat(float rotateX,
		float rotateY, float rotateZ);

	//// Rotate around a world-space point
	//void rotateAroundWorldPoint(const glm::vec3& axis, float angle, const glm::vec3& point);

	//// Rotate around a local-space point
	//void rotateAroundLocalPoint(const glm::vec3& axis, float angle, const glm::vec3& worldPoint);

	//void SetChild(Object3D child);
};

// Return the transform matrix
inline const glm::mat4& Object3D::getTransformMatrix() const {
	return mTransformMatrix;
}

// Set to the identity transform
inline void Object3D::setToIdentity() {
	mTransformMatrix = glm::mat4();
}

// Return the origin of object in world-space
inline glm::vec3 Object3D::getOrigin() const {
	return mTransformMatrix * glm::vec4(0.0, 0.0, 0.0, 1.0);
}

// Translate the object in world-space
inline void Object3D::translateWorld(const glm::vec3& v) {
	mTransformMatrix = glm::translate(glm::mat4(), v) * mTransformMatrix;
}

// Translate the object in local-space
inline void Object3D::translateLocal(const glm::vec3& v) {
	mTransformMatrix = mTransformMatrix * glm::translate(glm::mat4(), v);
}

//// Rotate the object in world-space
//inline void Object3D::rotateWorld(const glm::vec3& axis, float angle) {
//	mTransformMatrix = Matrix4::rotationMatrix(axis, angle) * mTransformMatrix;
//}

// Rotate the object in local-space
inline void Object3D::rotateLocal(const glm::vec3& axis, float angle) {
	/*mTransformMatrix = mTransformMatrix * Matrix4::rotationMatrix(axis, angle);*/

	mTransformMatrix = mTransformMatrix * glm::rotate(glm::mat4(), angle, axis);

}

inline void Object3D::rotateLocalQuat(float rotateX,
	float rotateY, float rotateZ)
{
	glm::vec3 eulerAngles = glm::vec3(rotateX, rotateY,
		rotateZ);
	glm::quat MyQuaternion = glm::quat(eulerAngles);

	mTransformMatrix = mTransformMatrix * glm::toMat4(MyQuaternion);
}

//// Rotate the object around a world-space point
//inline void Object3D::rotateAroundWorldPoint(const Vector3& axis, float angle,
//	const Vector3& worldPoint) {
//	mTransformMatrix = Matrix4::translationMatrix(worldPoint) * Matrix4::rotationMatrix(axis, angle)
//		* Matrix4::translationMatrix(-worldPoint) * mTransformMatrix;
//}

//// Rotate the object around a local-space point
//inline void Object3D::rotateAroundLocalPoint(const Vector3& axis, float angle,
//	const Vector3& worldPoint) {

//	// Convert the world point into the local coordinate system
//	Vector3 localPoint = mTransformMatrix.getInverse() * worldPoint;

//	mTransformMatrix = mTransformMatrix * Matrix4::translationMatrix(localPoint)
//		* Matrix4::rotationMatrix(axis, angle)
//		* Matrix4::translationMatrix(-localPoint);
//}

//void Object3D::SetChild(Object3D child)
//{
//
//}

