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
    unsigned int vertexOffset=0;
    unsigned int vertexCount=0;

    //offset for drawing buffers
    unsigned int bufferOffset=0;

    //textures
    GLuint texture = 0;
    bool textured = false;

    bool animatable=false;
    AnimationData animationData;
    std::vector<Joint> joints;

    std::vector<AnimationClip> clips;
    std::vector<std::string> clipNames;
    int currentClip=-1;

    void animate(float frame);
    void draw();
    //reset state to default
    void clear();
};

void graphicsInit();
void setAspectRatio(float ratio);
Model loadIQM(const char *filename);
GLuint loadTexture(const char *filename);
void uploadBuffers();
void clearBuffers();