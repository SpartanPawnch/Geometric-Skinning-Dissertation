#pragma once
#include<GL/glew.h>
#include<glm/mat3x3.hpp>
#include<glm/mat4x3.hpp>
#include<glm/vec3.hpp>
#include<vector>
#include<string>
struct Joint {
    glm::mat4 matrix;
    glm::mat4 inverse;
    int parent;
};
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
enum VertexWeightSet {
    VertexWeightSetBase = 0,
    VertexWeightSetBaseRigid = 1,
    VertexWeightSetAutoRigid = 2,
};
struct AnimationData {
    //regular skinning
    std::vector<glm::vec3> baseVertices;
    std::vector<glm::vec3> baseNormals;
    std::vector<glm::vec3> baseTangents;
    std::vector<Pose> poses;
    std::vector<float> vertexWeights;
    std::vector<int> weightIndices;
    unsigned int weightsPerVertex = 0;
    unsigned int posesPerFrame = 0;

    //delta mush
    bool deltaMushReady = false;
    std::vector<std::vector<int>>adjacency;
    std::vector<glm::vec3> averageNormals;
    std::vector<glm::vec3> deltas;

    //Initialization
    void generateWeightSets(Joint* joints);
    void prepareDeltaMush(unsigned int* faces, int count);

    //CPU Skinning
    void deformPositionLBS(glm::vec3* target, float frame, VertexWeightSet activeSet = VertexWeightSetBase);
    void deformPositionLBS(glm::vec3* target, float frame, const AnimationClip& clip,
        VertexWeightSet activeSet = VertexWeightSetBase);
    void deformNormalLBS(glm::vec3* target, float frame, VertexWeightSet activeSet = VertexWeightSetBase);
    void deformNormalLBS(glm::vec3* target, float frame, const AnimationClip& clip,
        VertexWeightSet activeSet = VertexWeightSetBase);

    void applyDeltaMush(glm::vec3* positions, glm::vec3* normal, float frame, VertexWeightSet activeSet = VertexWeightSetBase);
    void applyDeltaMush(glm::vec3* positions, glm::vec3* normal, float frame, const AnimationClip& clip, VertexWeightSet activeSet = VertexWeightSetBase);

    //GPU Skinning

    //copy vertex weights from active set to target buffer
    void copyWeights(glm::vec4* target, VertexWeightSet activeSet = VertexWeightSetBase);
    //copy bone indices for vertex weights
    void copyIndices(glm::ivec4* target, VertexWeightSet activeSet = VertexWeightSetBase);


    //upload the poses at an absolute frame
    void uploadPose(float frame, GLuint program);
    //upload the poses at the frame of a specific clip
    void uploadPose(float frame, GLuint program, const AnimationClip& clip);

    //Reset target to base value
    void resetVertices(glm::vec3* target);
    void resetNormals(glm::vec3* target);

    void clear();
};

void generateAdjacencyList(std::vector<std::vector<int>>& target, const unsigned int* faces, int faceCount, int vertexCount);
void smoothLaplacian(glm::vec3* source, glm::vec3* target, int count, std::vector<std::vector<int>>& adjacency, int iterations = 10);