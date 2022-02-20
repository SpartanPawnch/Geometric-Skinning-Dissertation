#include "camera.hpp"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
void Camera::rotateArcball(const glm::vec2 &cursorStart, const glm::vec2 &cursorEnd){
    //assume arcball radius is always 1
    glm::vec3 startPoint=glm::vec3(cursorStart,.0f);
    glm::vec3 endPoint=glm::vec3(cursorEnd,.0f);

    float startRad=startPoint.x*startPoint.x+startPoint.y*startPoint.y;
    float endRad=endPoint.x*endPoint.x+endPoint.y*endPoint.y;

    //compute point on arcball for initial cursor position
    if(startRad>1.0f){
        float scale=1/sqrt(startRad);
        startPoint*=scale;
    }
    else{
        startPoint.z=sqrt(1.0f-startRad);
    }

    //compute point on arcball for final cursor position
    if(endRad>1.0f){
        float scale=1/sqrt(endRad);
        endPoint*=scale;
    }
    else{
        endPoint.z=sqrt(1.0f-endRad);
    }

    glm::vec3 cross=glm::cross(startPoint,endPoint);
    glm::quat rotation(glm::dot(startPoint,endPoint),cross.x,cross.y,cross.z);
    
    eye=rotation*eye*glm::inverse(rotation);
    up=rotation*up*glm::inverse(rotation);

}
void Camera::pan(glm::vec2 cursorDelta){}
void Camera::zoom(float scrollDelta){}

glm::mat4 Camera::getMatrix(){
    return glm::lookAt(eye,center,up);
}