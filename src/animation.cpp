#include "animation.h"
#include <GL/glew.h>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <math.h>
#include <iostream>

// Initialization
void AnimationData::generateWeightSets(Joint* joints) {
    //rounded
    {
        for (int i = 0;i < baseVertices.size();i++) {
            //find vertex with maximum influence
            int maxInd = 0;
            for (int j = 1;j < weightsPerVertex;j++) {
                if (vertexWeights[i * weightsPerVertex + j] > vertexWeights[i * weightsPerVertex + maxInd])
                    maxInd = j;
            }
            for (int j = 0;j < weightsPerVertex;j++) {
                if (j == maxInd)
                    vertexWeights.push_back(1.0f);
                else
                    vertexWeights.push_back(.0f);
            }
        }
    }

    //automatic rigid bind
    {
        //calculate joint centers in world space
        glm::vec3* jointCenters = new glm::vec3[posesPerFrame];
        bool* isJointActive = new bool[posesPerFrame];
        for (int i = 0;i < posesPerFrame;i++) {
            jointCenters[i] = joints[i].matrix * glm::vec4(.0f, .0f, .0f, 1.0f);
            isJointActive[i] = false;
        }

        //ignore helper bones - only activate bones that were already bound
        for (int i = 0;i < baseVertices.size() * weightsPerVertex;i++) {
            isJointActive[weightIndices[i]] = true;
        }

        for (int i = 0;i < baseVertices.size();i++) {
            //find first active joint
            int startInd = 0;
            for (;startInd < posesPerFrame;startInd++) {
                if (isJointActive[startInd])
                    break;
            }

            //find closest joint
            float minDist = .0f;
            if (startInd < posesPerFrame)
                minDist = glm::length(jointCenters[startInd] - baseVertices[i]);
            int minInd = startInd;

            for (int j = startInd;j < posesPerFrame;j++) {
                float newDist = glm::length(jointCenters[j] - baseVertices[i]);
                if (newDist < minDist && isJointActive[j]) {
                    minInd = j;
                    minDist = newDist;
                }
            }


            //check if point is between the closest joint and its parent
            if (joints[minInd].parent >= 0) {
                glm::vec3 boneDir = glm::normalize(jointCenters[joints[minInd].parent] - jointCenters[minInd]);
                glm::vec3 vertDir = glm::normalize(baseVertices[i] - jointCenters[minInd]);
                float boneAngle = acos(glm::dot(boneDir, vertDir));

                if (boneAngle <= glm::radians(90.0f) && isJointActive[joints[minInd].parent]) {
                    //between joint and parent - assign to parent
                    minInd = joints[minInd].parent;
                }
            }

            //bind to closest joint
            bool weightAdded = false;
            weightIndices.push_back(minInd);
            vertexWeights.push_back(1.0f);
            for (int j = 1;j < weightsPerVertex;j++) {
                weightIndices.push_back(0);
                vertexWeights.push_back(.0f);
            }
        }
        delete[] jointCenters;
        delete[] isJointActive;
    }
}

// CPU skinning
void AnimationData::deformPositionLBS(glm::vec3* target, float frame, VertexWeightSet activeSet) {
    //resolve current pose
    Pose* currentFrame = &poses[((int)frame) * posesPerFrame];

    //resolve current weights and indices
    int weightSetLength = baseVertices.size() * weightsPerVertex;
    float* currentWeights = &vertexWeights[activeSet * weightSetLength];
    int* currentIndices = &weightIndices[(activeSet / 2) * weightSetLength];

    for (int i = 0;i < baseVertices.size();i++) {
        target[i] = glm::vec3(0.0f);
        for (int j = 0;j < weightsPerVertex;j++) {
            glm::vec3 deformContribution = currentFrame[currentIndices[i *
                weightsPerVertex + j]].rotscale * baseVertices[i] +
                currentFrame[currentIndices[i * weightsPerVertex + j]].translate;

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

    //resolve current weights and indices
    int weightSetLength = baseNormals.size() * weightsPerVertex;
    float* currentWeights = &vertexWeights[activeSet * weightSetLength];
    int* currentIndices = &weightIndices[(activeSet / 2) * weightSetLength];

    for (int i = 0;i < baseNormals.size();i++) {
        target[i] = glm::vec3(0.0f);
        for (int j = 0;j < weightsPerVertex;j++) {
            glm::vec3 deformContribution = currentFrame[currentIndices[i *
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
void AnimationData::copyWeights(glm::vec4* target, VertexWeightSet activeSet) {
    const int weightSetLength = baseNormals.size() * weightsPerVertex;
    float* currentWeights = &vertexWeights[activeSet * weightSetLength];
    for (int i = 0;i < baseVertices.size();i++) {
        glm::vec4 weight(.0f);
        for (int j = 0;j < weightsPerVertex && j < 4;j++) {
            weight[j] = currentWeights[i * weightsPerVertex + j];
        }
        target[i] = weight;
    }
}
void AnimationData::copyIndices(glm::ivec4* target, VertexWeightSet activeSet) {
    const int weightSetLength = baseNormals.size() * weightsPerVertex;
    int* activeIndices = &weightIndices[(activeSet / 2) * weightSetLength];
    for (int i = 0;i < baseVertices.size();i++) {
        glm::ivec4 index(0);
        for (int j = 0;j < weightsPerVertex && j < 4;j++) {
            index[j] = activeIndices[i * weightsPerVertex + j];
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

//create adjacency list based on list of triagles
void generateAdjacencyList(std::vector<std::vector<int>>& target, const int* faces, int faceCount, int vertexCount) {
    target.clear();
    target.resize(vertexCount);
    for (int i = 0;i < faceCount;i++) {
        //update neighbours for all three vertices
        for (int j = 0;j < 3;j++) {
            //check if vertices are already added to list
            bool found = false;
            for (int k = 0;k < target[faces[3 * i + j]].size();k++) {
                if (target[faces[3 * i + j]][k] == faces[3 * i + (j + 1) % 3]) {
                    found = true;
                    break;
                }
            }
            if (!found)
                target[faces[3 * i + j]].push_back(faces[3 * i + (j + 1) % 3]);

            found = false;
            for (int k = 0;k < target[faces[3 * i + j]].size();k++) {
                if (target[faces[3 * i + j]][k] == faces[3 * i + (j + 2) % 3]) {
                    found = true;
                    break;
                }
            }
            if (!found)
                target[faces[3 * i + j]].push_back(faces[3 * i + (j + 2) % 3]);
        }
    }
}

void smoothLaplacian(glm::vec3* source, glm::vec3* target) {}

void AnimationData::prepareDeltaMush(int* faces, int count) {
    std::vector<std::vector<int>>target;
    generateAdjacencyList(target, faces, count, baseVertices.size());

    //generate deltas
    //smoothLaplacian
}