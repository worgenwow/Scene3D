#pragma once
#include <key_data_struct.h>
#include <openglMaths.h>

class Camera {
  private:
    oglm::vec3 mPosition;
    oglm::vec3 mFrontVector;
    oglm::vec3 mRightVector;
    oglm::vec3 mUpVector;
    const oglm::vec3 mWorldUp;

    const float mCameraSpeed;
    const float mAngleChange;
    const float mAngleBound;
    
    bool mFlipped = false;

    float mYaw;
    float mPitch;

    void updateVectors();
  public:
    Camera();

    void flipDirection(bool flip);

    void updatePos(KeyData *keyData, float deltaTime);
    void updateAngle(oglm::vec2 *mouseChange, float deltaTime);
    oglm::mat4 getViewMatrix();
    oglm::vec3 getPosition() const;
    oglm::vec3 getFrontVector();
};