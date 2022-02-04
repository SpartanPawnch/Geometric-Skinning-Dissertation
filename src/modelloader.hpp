#pragma once
#include<GL/glew.h>
#include<vector>
#include<glm/mat4x4.hpp>
#include<glm/gtc/quaternion.hpp>
#include<glm/gtx/quaternion.hpp>

struct Pose
{
    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 scale;
};

struct Model {
    unsigned int vertexOffset;
    unsigned int vertexCount;

    GLuint texture = 0;
    bool textured = false;
    std::vector<glm::mat3x4> jointsMatrices;
    std::vector<glm::mat3x4> inverseJointMatrices;

    std::vector<Pose> poses;


    void draw();
};

void graphicsInit();
Model loadIQM(const char *filename);
GLuint loadTexture(const char *filename);
void uploadBuffers();