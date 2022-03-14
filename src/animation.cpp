#include "animation.h"
#include <GL/glew.h>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <math.h>
#include <iostream>


// CPU skinning
void AnimationData::deformPositionLBS(glm::vec3* target, float frame, VertexWeightSet activeSet) {
    //resolve current pose
    Pose* currentFrame = &poses[((int)frame) * posesPerFrame];

    //resolve current weights
    int weightSetLength = baseVertices.size() * weightsPerVertex;
    float* currentWeights = &vertexWeights[activeSet * weightSetLength];
    for (int i = 0;i < baseVertices.size();i++) {
        target[i] = glm::vec3(0.0f);
        for (int j = 0;j < weightsPerVertex;j++) {
            glm::vec3 deformContribution = currentFrame[weightIndices[i *
                weightsPerVertex + j]].rotscale * baseVertices[i] +
                currentFrame[weightIndices[i * weightsPerVertex + j]].translate;

            target[i] += currentWeights[i * weightsPerVertex + j] * deformContribution;
        }
    }
}
void AnimationData::deformPositionLBS(glm::vec3* target, float frame, const AnimationClip& clip, VertexWeightSet activeSet) {
    float absoluteFrame = clip.offset + fmodf(frame, clip.length);
    deformPositionLBS(target, absoluteFrame, activeSet);
}
void AnimationData::deformNormalLBS(glm::vec3* target, float frame, VertexWeightSet activeSet) {
    //resolve current pose
    Pose* currentFrame = &poses[((int)frame) * posesPerFrame];

    //resolve current weights
    int weightSetLength = baseNormals.size() * weightsPerVertex;
    float* currentWeights = &vertexWeights[activeSet * weightSetLength];

    for (int i = 0;i < baseNormals.size();i++) {
        target[i] = glm::vec3(0.0f);
        for (int j = 0;j < weightsPerVertex;j++) {
            glm::vec3 deformContribution = currentFrame[weightIndices[i *
                weightsPerVertex + j]].rotscale * baseNormals[i];

            target[i] += currentWeights[i * weightsPerVertex + j] * deformContribution;
        }
    }
}
void AnimationData::deformNormalLBS(glm::vec3* target, float frame, const AnimationClip& clip, VertexWeightSet activeSet) {
    float absoluteFrame = clip.offset + fmodf(frame, clip.length);
    deformNormalLBS(target, absoluteFrame, activeSet);
}



//GPU Skinning
void AnimationData::copyWeights(glm::vec4* target, VertexWeightSet set) {
    int weightSetLength = baseNormals.size() * weightsPerVertex;
    float* currentWeights = &vertexWeights[set * weightSetLength];
    for (int i = 0;i < baseVertices.size();i++) {
        glm::vec4 weight(.0f);
        for (int j = 0;j < weightsPerVertex && j < 4;j++) {
            weight[j] = vertexWeights[i * weightsPerVertex + j];
        }
        target[i] = weight;
    }
}
void AnimationData::copyIndices(glm::ivec4* target) {
    for (int i = 0;i < baseVertices.size();i++) {
        glm::ivec4 index(0);
        for (int j = 0;j < weightsPerVertex && j < 4;j++) {
            index[j] = weightIndices[i * weightsPerVertex + j];
        }
        target[i] = index;
    }
}

void AnimationData::uploadPose(float frame, GLuint program) {
    Pose* currentFrame = &poses[((int)frame) * posesPerFrame];
    GLuint location = glGetUniformLocation(program, "poses");
    glUniformMatrix4x3fv(location, posesPerFrame, false, (GLfloat*)&currentFrame[0]);
}
void AnimationData::uploadPose(float frame, GLuint program, const AnimationClip& clip) {
    float absoluteFrame = clip.offset + fmodf(frame, clip.length);
    uploadPose(absoluteFrame, program);
}

void AnimationData::resetVertices(glm::vec3* target) {
    for (int i = 0;i < baseVertices.size();i++)
        target[i] = baseVertices[i];
}

void AnimationData::resetNormals(glm::vec3* target) {
    for (int i = 0;i < baseNormals.size();i++)
        target[i] = baseNormals[i];
}

void AnimationData::clear() {
    baseVertices.clear();
    baseNormals.clear();
    poses.clear();
    vertexWeights.clear();
    weightIndices.clear();
    weightsPerVertex = 0;
    posesPerFrame = 1;
}
