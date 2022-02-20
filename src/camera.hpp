#pragma once
#include<glm/gtc/quaternion.hpp>
#include<glm/vec3.hpp>

struct Camera{
    glm::vec3 eye;
    glm::vec3 center;
    glm::vec3 up;
    float screenX,screenY;

    Camera(){
        eye=glm::vec3(.0f,.0f,1.0f);
        center=glm::vec3(.0f);
        up=glm::vec3(.0f,1.0f,.0f);
        screenX=1.0f;
        screenY=1.0f;
    }

    //standard camera manipulations
    void rotateArcball(const glm::vec2 &cursorStart, const glm::vec2 &cursorEnd);
    void pan(glm::vec2 cursorDelta);
    void zoom(float scrollDelta);
};
