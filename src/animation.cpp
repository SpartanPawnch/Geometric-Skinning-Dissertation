#include "animation.h"
#include <glm/mat4x4.hpp>
#include <iostream>

void AnimationData::deformPositionLBS(glm::vec3 *target, float frame){
    glm::mat4 *currentFrame=&poses[((int)frame)*posesPerFrame];

    for(int i=0;i<baseVertices.size();i++){
        target[i]=glm::vec3(0.0f);
        for(int j=0;j<weightsPerVertex;j++){
            glm::vec3 deformContribution=currentFrame[weightIndices[i*weightsPerVertex+j]]*
                glm::vec4(baseVertices[i],1.0f);
            
            target[i]+=vertexWeights[i*weightsPerVertex+j]*deformContribution;
        }
    }
}
void AnimationData::deformNormalLBS(glm::vec3 *target, float frame){
    glm::mat4 *currentFrame=&poses[((int)frame)*posesPerFrame];

    for(int i=0;i<baseNormals.size();i++){
        target[i]=glm::vec3(0.0f);
        for(int j=0;j<weightsPerVertex;j++){
            glm::vec3 deformContribution=glm::mat3(currentFrame[weightIndices[i*
                weightsPerVertex+j]])*baseNormals[i];
            
            target[i]+=vertexWeights[i*weightsPerVertex+j]*deformContribution;
        }
    }
}