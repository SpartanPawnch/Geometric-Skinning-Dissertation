#include "camera.hpp"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <iostream>

void Camera::rotateArcball(const glm::vec2 &cursorStart, const glm::vec2 &cursorEnd){
    //assume arcball radius is always the smaller screen dimension
    glm::vec2 scaledStart=2.0f*glm::vec2(cursorStart.x/screenWidth-.5f,-cursorStart.y/screenHeight+.5f);
    glm::vec2 scaledEnd=2.0f*glm::vec2(cursorEnd.x/screenWidth-.5f,-cursorEnd.y/screenHeight+.5f);
    glm::vec3 startPoint=glm::vec3(scaledStart,.0f);
    glm::vec3 endPoint=glm::vec3(scaledEnd,.0f);


    //compute point on arcball for initial cursor position
    float startRad=glm::dot(scaledStart,scaledStart);
    if(startRad>1.0f){
        //outside ball - project onto surface
        startPoint=glm::vec3(glm::normalize(scaledStart),.0f);
    }
    else{
        //inside ball - compute distance
        startPoint.z=sqrt(1.0f-startRad);
    }


    //compute point on arcball for final cursor position
    float endRad=glm::dot(scaledEnd,scaledEnd);
    if(endRad>1.0f){
        // outside ball - project onto surface
        endPoint=glm::vec3(glm::normalize(scaledEnd),.0f);
    }
    else{
        //inside ball - compute distance
        endPoint.z=sqrt(1.0f-endRad);
    }


    glm::quat rotation=glm::normalize(glm::quat(.0f,endPoint)*glm::quat(.0f,startPoint));
    
    orientation=glm::normalize(rotation*orientation);
}
void Camera::pan(glm::vec2 cursorDelta){}
void Camera::zoom(float scrollDelta){}

glm::mat4 Camera::getMatrix(){
    return glm::perspective(glm::radians(45.0f),screenWidth/screenHeight,clipNear,clipFar)*
        glm::translate(translation)*glm::mat4_cast(orientation)*glm::translate(-center);
}