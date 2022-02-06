#include "animation.h"
#include <glm/mat4x4.hpp>
#include <iostream>

void animateLBS(const glm::vec3 *baseVertices, const int vertexCount, glm::vec3 *deformedVertices,
    const glm::mat4 *poses, const float *weights, const unsigned int *indices, 
    const int weightsPerVertex){
    
    for(int i=0;i<vertexCount;i++){
        deformedVertices[i]=glm::vec3(0.0f);
        for(int j=0;j<weightsPerVertex;j++){
            glm::vec3 deformContribution=poses[indices[i*weightsPerVertex+j]]*
                glm::vec4(baseVertices[i],1.0f);
            
            deformedVertices[i]+=weights[i*weightsPerVertex+j]*deformContribution;
        }
    }
}