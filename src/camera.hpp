#pragma once
#include<glm/gtc/quaternion.hpp>
#include<glm/vec3.hpp>
#include<glm/mat4x4.hpp>
#include<glm/gtx/transform.hpp>

struct Camera{
    private:
    //maintain internal view matrix components for easier camera control implementation
    glm::quat orientation;
    glm::vec3 translation;
    glm::vec3 center;

    glm::mat4 cameraInverse(){
        return glm::inverse(glm::translate(translation)*glm::mat4_cast(orientation)*
            glm::translate(-center));
    }

    public:


    float screenWidth,screenHeight;
    float clipNear,clipFar;

    Camera(const glm::vec3 &_eye, const glm::vec3 &_center, const glm::vec3 &_up){
        //standard view matrix setup
        glm::vec3 viewDir=_center-_eye;
        glm::vec3 zAxis=glm::normalize(viewDir);
        glm::vec3 xAxis=glm::normalize(glm::cross(zAxis,glm::normalize(_up)));
        glm::vec3 yAxis=glm::normalize(glm::cross(xAxis,zAxis));
        xAxis=glm::normalize(glm::cross(zAxis,yAxis));

        center=_center;
        translation=glm::vec3(.0f,.0f,-glm::length(viewDir));
        orientation=glm::normalize(glm::quat_cast(glm::transpose(glm::mat3(xAxis,yAxis,-zAxis))));

        //set perspective defaults
        screenWidth=1.0f;
        screenHeight=1.0f;
        clipNear=1.0f;
        clipFar=10.0f;
    }

    Camera(){
        glm::vec3 eye=glm::vec3(.0f,.0f,1.0f);
        center=glm::vec3(.0f);
        glm::vec3 up=glm::vec3(.0f,1.0f,.0f);

        Camera(eye,center,up);
    }

    //standard camera manipulations
    void rotateArcball(const glm::vec2 &cursorStart, const glm::vec2 &cursorEnd);
    void pan(glm::vec2 cursorDelta);
    void zoom(float scrollDelta);

    //components
    glm::mat4 getMatrix();
    glm::vec3 getEye(){
        return glm::vec3(cameraInverse()*glm::vec4(.0f,.0f,.0f,1.0f));
    }
    glm::vec3 getCenter(){
        return center;
    }
    glm::vec3 getUp(){
        return glm::vec3(cameraInverse()*glm::vec4(.0f,1.0f,.0f,.0f));
    }
};
