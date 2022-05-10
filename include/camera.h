#pragma once
#include <glm/glm.hpp>
#include <key_data_struct.h>

class Camera {
  private:
    glm::vec3 mPosition;
    glm::vec3 mFrontVector;
    glm::vec3 mRightVector;
    glm::vec3 mUpVector;
    const glm::vec3 mWorldUp;

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
    void updateAngle(glm::vec2 *mouseChange, float deltaTime);
    glm::mat4 getViewMatrix();
    glm::vec3 getPosition() const;
    glm::vec3 getFrontVector();
};