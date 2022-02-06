#pragma once
#include<glm/mat4x4.hpp>
void animateLBS(const glm::vec3 *baseVertices, const int vertexCount, glm::vec3 *deformedVertices,
    const glm::mat4 *poses, const float *weights, const unsigned int *indices, 
    const int weightsPerVertex);