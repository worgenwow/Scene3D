#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <camera.h>
#include <math.h>

Camera::Camera() : 
mCameraSpeed(3),
mWorldUp(glm::vec3(0.0f, 1.0f,  0.0f)),
mAngleChange(8 * M_PI/180),
mAngleBound(89 * M_PI/180) {
  mPosition   = glm::vec3(0.0f, 0.0f,  3.0f);
  mFrontVector = glm::vec3(0.0f, 0.0f, -1.0f);

  mYaw = -M_PI/2;
  mPitch = 0;
}

void Camera::flipDirection(bool flip) {
  mFlipped = flip;
  updateVectors();
}

// moves the camera round
void Camera::updatePos(KeyData *keyData, float deltaTime) {
  if(keyData->wKeyDown) {
    mPosition += mFrontVector * mCameraSpeed * deltaTime;
  }
  if(keyData->dKeyDown) {
    mPosition += mRightVector * mCameraSpeed * deltaTime;
  }
  if(keyData->sKeyDown) {
    mPosition -= mFrontVector * mCameraSpeed * deltaTime;
  }
  if(keyData->aKeyDown) {
    mPosition -= mRightVector * mCameraSpeed * deltaTime;
  }
  if(keyData->spaceKeyDown) {
    mPosition += mUpVector * mCameraSpeed * deltaTime;
  }
  if(keyData->cKeyDown) {
    mPosition -= mUpVector * mCameraSpeed * deltaTime;
  }
}

// rotates the camera
void Camera::updateAngle(glm::vec2 *mouseChange, float deltaTime) {
  mYaw   += mouseChange->x * mAngleChange * deltaTime;
  mPitch -= mouseChange->y * mAngleChange * deltaTime;

  if(mPitch > mAngleBound) {
    mPitch = mAngleBound;
  } else if(mPitch < -mAngleBound) {
    mPitch = -mAngleBound;
  }
}

glm::mat4 Camera::getViewMatrix() {
  glm::mat4 view;

  updateVectors();
  view = glm::lookAt(mPosition,                // position vector
                     mPosition + mFrontVector, // target position vector
                     mUpVector);                // up vector

  return view;
}

void Camera::updateVectors() {
  mFrontVector = glm::vec3(cos(mYaw) * cos(mPitch), sin(mPitch), sin(mYaw) * cos(mPitch));
  int flip = mFlipped ? -1 : 1;
  mFrontVector = glm::vec3(flip) * glm::normalize(mFrontVector);

  mRightVector = glm::normalize(glm::cross(mFrontVector, mWorldUp));
  mUpVector = glm::normalize(glm::cross(mRightVector, mFrontVector));
}

glm::vec3 Camera::getPosition() const {
  return mPosition;
}

glm::vec3 Camera::getFrontVector() {
  updateVectors();
  
  return mFrontVector;
}