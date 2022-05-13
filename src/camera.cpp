#include <camera.h>
#include <math.h>
#include <openglMaths.h>

Camera::Camera() : 
mCameraSpeed(3),
mWorldUp(oglm::vec3(0.0f, 1.0f,  0.0f)),
mAngleChange(8 * M_PI/180),
mAngleBound(89 * M_PI/180) {
  mPosition   = oglm::vec3(0.0f, 0.0f,  3.0f);
  mFrontVector = oglm::vec3(0.0f, 0.0f, -1.0f);

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
void Camera::updateAngle(oglm::vec2 *mouseChange, float deltaTime) {
  mYaw   += mouseChange->x * mAngleChange * deltaTime;
  mPitch -= mouseChange->y * mAngleChange * deltaTime;

  if(mPitch > mAngleBound) {
    mPitch = mAngleBound;
  } else if(mPitch < -mAngleBound) {
    mPitch = -mAngleBound;
  }
}

oglm::mat4 Camera::getViewMatrix() {
  oglm::mat4 view;

  updateVectors();
  view = oglm::lookAt(mPosition,                // position vector
                     mPosition + mFrontVector, // target position vector
                     mUpVector);                // up vector

  return view;
}

void Camera::updateVectors() {
  mFrontVector = oglm::vec3(cos(mYaw) * cos(mPitch), sin(mPitch), sin(mYaw) * cos(mPitch));
  int flip = mFlipped ? -1 : 1;
  mFrontVector = oglm::vec3(flip) * oglm::normalize(mFrontVector);

  mRightVector = oglm::normalize(oglm::cross(mFrontVector, mWorldUp));
  mUpVector = oglm::normalize(oglm::cross(mRightVector, mFrontVector));
}

oglm::vec3 Camera::getPosition() const {
  return mPosition;
}

oglm::vec3 Camera::getFrontVector() {
  updateVectors();
  
  return mFrontVector;
}