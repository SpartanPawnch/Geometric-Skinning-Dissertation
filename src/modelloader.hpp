#pragma once
#include<GL/glew.h>
#include<vector>
#include<glm/mat4x4.hpp>
#include<glm/gtc/quaternion.hpp>
#include<glm/gtx/quaternion.hpp>

struct Joint{
    glm::mat4 matrix;
    glm::mat4 inverse;
};

struct Model {
    //rendering
    unsigned int vertexOffset;
    unsigned int vertexCount;

    //textures
    GLuint texture = 0;
    bool textured = false;


    //animation
    //TODO refactor
    std::vector<glm::vec3> baseVertices;

    std::vector<Joint> joints;

    unsigned int weightsPerVertex=0;
    std::vector<float> vertexWeights; //weights are unsigned bytes
    std::vector<unsigned int> weightIndices; //index of joint weight is applied to

    unsigned int posesPerFrame=0;
    std::vector<glm::mat4> poses;

    void animate(float frame);
    void draw();
};

void graphicsInit();
void setAspectRatio(float ratio);
Model loadIQM(const char *filename);
GLuint loadTexture(const char *filename);
void uploadBuffers();