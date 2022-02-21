#include "camera.hpp"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <iostream>


static constexpr glm::vec2 toRelativeScreenCoordinates(const glm::vec2& v, float screenWidth, float screenHeight) {
    glm::vec2 scaled = 2.0f * glm::vec2(v.x / screenWidth - .5f, -v.y / screenHeight + .5f);
    return scaled;
}

void Camera::rotateArcball(const glm::vec2& cursorStart, const glm::vec2& cursorEnd) {
    //assume arcball radius is always the smaller screen dimension
    glm::vec2 scaledStart = toRelativeScreenCoordinates(cursorStart, screenWidth, screenHeight);
    glm::vec2 scaledEnd = toRelativeScreenCoordinates(cursorEnd, screenWidth, screenHeight);
    glm::vec3 startPoint = glm::vec3(scaledStart, .0f);
    glm::vec3 endPoint = glm::vec3(scaledEnd, .0f);


    //compute point on arcball for initial cursor position
    float startRad = glm::dot(scaledStart, scaledStart);
    if (startRad > 1.0f) {
        //outside ball - project onto surface
        startPoint = glm::vec3(glm::normalize(scaledStart), .0f);
    }
    else {
        //inside ball - compute distance
        startPoint.z = sqrt(1.0f - startRad);
    }


    //compute point on arcball for final cursor position
    float endRad = glm::dot(scaledEnd, scaledEnd);
    if (endRad > 1.0f) {
        // outside ball - project onto surface
        endPoint = glm::vec3(glm::normalize(scaledEnd), .0f);
    }
    else {
        //inside ball - compute distance
        endPoint.z = sqrt(1.0f - endRad);
    }


    glm::quat rotation = glm::normalize(glm::quat(.0f, endPoint) * glm::quat(.0f, startPoint));

    orientation = glm::normalize(rotation * orientation);
}
void Camera::pan(glm::vec2 cursorDelta) {

    glm::vec2 cursorScaled = glm::vec2(2.0f * cursorDelta.x / screenWidth, 2.0f * cursorDelta.y / screenHeight);
    //revert to world coordinates
    glm::vec3 worldTranslation = cameraInverse() * glm::vec4(cursorScaled.x, cursorScaled.y, .0f, .0f);
    center += worldTranslation;
}
void Camera::zoom(float scrollDelta) {
    cameraZoom = glm::clamp(cameraZoom - scrollDelta, .0001f, 10.0f);
}

glm::mat4 Camera::getMatrix() {
    return glm::perspective(glm::radians(45.0f), screenWidth / screenHeight, cameraZoom * clipNear, cameraZoom * clipFar) *
        glm::translate(cameraZoom * translation) * glm::mat4_cast(orientation) * glm::translate(-center);
}