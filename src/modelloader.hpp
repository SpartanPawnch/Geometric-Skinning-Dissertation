#pragma once
#include<GL/glew.h>
#include<vector>
#include<glm/mat4x4.hpp>
#include<glm/gtc/quaternion.hpp>
#include<glm/gtx/quaternion.hpp>

#include "animation.h"

struct Joint{
    glm::mat4 matrix;
    glm::mat4 inverse;
};

struct Model {
    //rendering
    unsigned int vertexOffset;
    unsigned int vertexCount;

    //offset for drawing buffers
    unsigned int bufferOffset;

    //textures
    GLuint texture = 0;
    bool textured = false;

    AnimationData animationData;
    std::vector<Joint> joints;

    void animate(float frame);
    void draw();
};

void graphicsInit();
void setAspectRatio(float ratio);
Model loadIQM(const char *filename);
GLuint loadTexture(const char *filename);
void uploadBuffers();