#include "animation.h"
#include <glm/mat4x4.hpp>
#include <iostream>

void AnimationData::deformPositionLBS(glm::vec3 *target, float frame){
    Pose *currentFrame=&poses[((int)frame)*posesPerFrame];

    for(int i=0;i<baseVertices.size();i++){
        target[i]=glm::vec3(0.0f);
        for(int j=0;j<weightsPerVertex;j++){
            glm::vec3 deformContribution=currentFrame[weightIndices[i*
                weightsPerVertex+j]].rotscale*baseVertices[i]+
                currentFrame[weightIndices[i*weightsPerVertex+j]].translate;
            
            target[i]+=vertexWeights[i*weightsPerVertex+j]*deformContribution;
        }
    }
}
void AnimationData::deformPositionLBS(glm::vec3 *target, float frame, const AnimationClip &clip){
    float absoluteFrame=clip.offset+fmodf(frame,clip.length);
    deformPositionLBS(target,absoluteFrame);
}
void AnimationData::deformNormalLBS(glm::vec3 *target, float frame){
    Pose *currentFrame=&poses[((int)frame)*posesPerFrame];

    for(int i=0;i<baseNormals.size();i++){
        target[i]=glm::vec3(0.0f);
        for(int j=0;j<weightsPerVertex;j++){
            glm::vec3 deformContribution=currentFrame[weightIndices[i*
                weightsPerVertex+j]].rotscale*baseNormals[i];
            
            target[i]+=vertexWeights[i*weightsPerVertex+j]*deformContribution;
        }
    }
}
void AnimationData::deformNormalLBS(glm::vec3 *target, float frame, const AnimationClip &clip){
    float absoluteFrame=clip.offset+fmodf(frame,clip.length);
    deformNormalLBS(target,absoluteFrame);
}
void AnimationData::clear(){
    baseVertices.clear();
    baseNormals.clear();
    poses.clear();
    vertexWeights.clear();
    weightIndices.clear();
    weightsPerVertex=0;
    posesPerFrame=0;
}