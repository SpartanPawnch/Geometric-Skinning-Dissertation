#pragma once
#include<vector>
#include<glm/mat4x4.hpp>
#include<glm/gtc/quaternion.hpp>
#include<glm/gtx/quaternion.hpp>
struct Joint
{
    glm::mat4x4 offset;
    int parent;
};


struct Pose
{
    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 scale;
};

struct Model {
    unsigned int vertexOffset;
    unsigned int vertexCount;

    std::vector<Joint> joints;
    int rootJoint;
    std::vector<Pose> poses;


    void draw();
};

void graphicsInit();
Model loadIQM(const char *filename);
void uploadBuffers();