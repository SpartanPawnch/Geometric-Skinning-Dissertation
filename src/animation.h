#pragma once
#include<glm/mat4x4.hpp>
#include<vector>
struct AnimationData{
    std::vector<glm::vec3> baseVertices;
    std::vector<glm::vec3> baseNormals;
    std::vector<glm::mat4> poses;
    std::vector<float> vertexWeights;
    std::vector<int> weightIndices;
    unsigned int weightsPerVertex=0;
    unsigned int posesPerFrame=0;


    void deformPositionLBS(glm::vec3 *target,float frame);
    void deformNormalLBS(glm::vec3 *target,float frame);
};
