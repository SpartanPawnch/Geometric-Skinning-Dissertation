#pragma once
#include<GL/glew.h>
#include<glm/mat3x3.hpp>
#include<glm/mat4x3.hpp>
#include<glm/vec3.hpp>
#include<vector>
#include<string>
struct Pose {
    glm::mat3 rotscale;
    glm::vec3 translate;
    operator glm::mat4x3() const {
        return glm::mat4x3(rotscale[0], rotscale[1], rotscale[2], translate);
    }
};
struct AnimationClip {
    unsigned int offset;
    unsigned int length;
    float framerate;
};
struct AnimationData {
    std::vector<glm::vec3> baseVertices;
    std::vector<glm::vec3> baseNormals;
    std::vector<Pose> poses;
    std::vector<float> vertexWeights;
    std::vector<int> weightIndices;
    unsigned int weightsPerVertex = 0;
    unsigned int posesPerFrame = 0;


    void deformPositionLBS(glm::vec3* target, float frame);
    void deformPositionLBS(glm::vec3* target, float frame, const AnimationClip& clip);
    void deformNormalLBS(glm::vec3* target, float frame);
    void deformNormalLBS(glm::vec3* target, float frame, const AnimationClip& clip);
    void uploadPose(float frame, GLuint program);
    void uploadPose(float frame, GLuint program, const AnimationClip& clip);
    void clear();
};
