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
        target[i] = glm::normalize(target[i]);
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
    baseTangents.clear();
    poses.clear();
    vertexWeights.clear();
    weightIndices.clear();
    for (int i = 0;i < adjacency.size();i++) {
        adjacency[i].clear();
    }
    adjacency.clear();
    deltas.clear();
    deltaMushReady = false;
    weightsPerVertex = 0;
    posesPerFrame = 1;
}

//create adjacency list based on list of triagles
void generateAdjacencyList(std::vector<std::vector<int>>& target, const unsigned int* faces, int faceCount, int vertexCount) {
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

void smoothLaplacian(glm::vec3* source, glm::vec3* target, int count, std::vector<std::vector<int>>& adjacency, int iterations) {
    //initial iteration uses source
    for (int i = 0;i < count;i++) {
        glm::vec3 sum(.0f);
        for (int j = 0;j < adjacency[i].size();j++) {
            sum += source[adjacency[i][j]];
        }

        //simplest option - fully replace with average of neighbours (no blending)
        target[i] = (1.0f / adjacency[i].size()) * sum;
    }

    iterations--;

    //further iterations copy the previous iteration's results
    if (iterations > 0) {
        glm::vec3* previousIteration = new glm::vec3[count];
        while (iterations > 0) {
            for (int i = 0;i < count;i++) {
                previousIteration[i] = target[i];
            }
            for (int i = 0;i < count;i++) {
                glm::vec3 sum(.0f);
                for (int j = 0;j < adjacency[i].size();j++) {
                    sum += previousIteration[adjacency[i][j]];
                }

                //simplest option - fully replace with average of neighbours (no blending)
                target[i] = (1.0f / adjacency[i].size()) * sum;
            }
            iterations--;
        }
        delete[] previousIteration;
    }
}

void AnimationData::prepareDeltaMush(unsigned int* faces, int count) {
    //prepare vertex data for smoothing
    {
        //generates simple adjacency only, doesn't compute duplicates
        generateAdjacencyList(adjacency, faces, count, baseVertices.size());

        //we can have multiple copies of each position with different normals/tangents
        //need to average out for Laplacian and fix adjacency


        bool* consolidated = new bool[baseVertices.size()];
        for (int i = 0;i < baseVertices.size();i++) {
            consolidated[i] = false;
        }

        for (int i = 0;i < baseVertices.size();i++) {
            if (!consolidated[i]) {
                std::vector<int> duplicateIndices;
                duplicateIndices.push_back(i);
                //find all vertices with identical positions
                for (int j = i + 1;j < baseVertices.size();j++) {
                    if (baseVertices[i] == baseVertices[j]) {
                        duplicateIndices.push_back(j);
                    }
                }

                //find sums for normals and tangent
                glm::vec3 normalSum = glm::vec3(.0f);
                glm::vec3 tangentSum = glm::vec3(.0f);
                for (int j = 0;j < duplicateIndices.size();j++) {
                    normalSum += glm::normalize(baseNormals[duplicateIndices[j]]);
                    tangentSum += glm::normalize(baseTangents[duplicateIndices[j]]);
                }

                //propagate averages and consolidate adjacency lists
                averageNormals.resize(baseNormals.size());
                for (int j = 0;j < duplicateIndices.size();j++) {
                    //regular per-face normals are used for drawing, need to use separate variable
                    averageNormals[duplicateIndices[j]] = glm::normalize(normalSum);
                    //we can directly overwrite the tangents, as we no longer need the originals
                    baseTangents[duplicateIndices[j]] = glm::normalize(tangentSum);

                    //propagate adjacencies
                    for (int k = j + 1;k < duplicateIndices.size();k++) {
                        //propagate adjacencies from duplicate j to duplicate k
                        for (int l = 0;l < adjacency[duplicateIndices[j]].size();l++) {
                            bool found = false;
                            for (int m = 0;m < adjacency[duplicateIndices[k]].size();m++) {
                                if (baseVertices[adjacency[duplicateIndices[j]][l]] == baseVertices[adjacency[duplicateIndices[k]][m]]) {
                                    found = true;
                                    break;
                                }
                            }
                            //not found => add to list
                            if (!found)
                                adjacency[duplicateIndices[k]].push_back(adjacency[duplicateIndices[j]][l]);
                        }
                        //propagate adjacencies from duplicate k to duplicate j
                        for (int l = 0;l < adjacency[duplicateIndices[k]].size();l++) {
                            bool found = false;
                            for (int m = 0;m < adjacency[duplicateIndices[j]].size();m++) {
                                if (baseVertices[adjacency[duplicateIndices[k]][l]] == baseVertices[adjacency[duplicateIndices[j]][m]]) {
                                    found = true;
                                    break;
                                }
                            }
                            //not found => add to list
                            if (!found)
                                adjacency[duplicateIndices[j]].push_back(adjacency[duplicateIndices[k]][l]);
                        }
                    }
                    consolidated[duplicateIndices[j]] = true;
                }
            }
        }

        delete[] consolidated;

    }

    //solve rest pose for offsets
    {
        //create a smoothed set of vertices
        std::vector<glm::vec3> smoothedVertices;
        smoothedVertices.resize(baseVertices.size());
        smoothLaplacian(&baseVertices[0], &smoothedVertices[0], baseVertices.size(), adjacency);
        std::vector<glm::vec3> smoothedNormals;
        smoothedNormals.resize(baseNormals.size());
        smoothLaplacian(&averageNormals[0], &smoothedNormals[0], baseNormals.size(), adjacency);
        std::vector<glm::vec3> smoothedTangents;
        smoothedTangents.resize(baseTangents.size());
        smoothLaplacian(&baseTangents[0], &smoothedTangents[0], baseTangents.size(), adjacency);

        //calculate deltas
        deltas.resize(baseVertices.size());
        for (int i = 0;i < baseVertices.size();i++) {
            glm::vec3 bitangent = glm::cross(glm::normalize(smoothedNormals[i]), glm::normalize(smoothedTangents[i]));
            glm::mat3 restCoordinates = glm::inverse(glm::mat3(glm::normalize(smoothedTangents[i]), glm::normalize(smoothedNormals[i]),
                glm::normalize(bitangent)));
            deltas[i] = restCoordinates * (baseVertices[i] - smoothedVertices[i]);
        }

    }
    deltaMushReady = true;
}

void AnimationData::applyDeltaMush(glm::vec3* positions, glm::vec3* normals, float frame, VertexWeightSet activeSet) {

    //resolve current pose
    Pose* currentFrame = &poses[((int)frame) * posesPerFrame];

    //apply LBS to tangents and average normals- not used anywhere else
    std::vector<glm::vec3> skinnedTangents;
    skinnedTangents.resize(baseTangents.size());
    std::vector<glm::vec3> skinnedAverageNormals;
    skinnedAverageNormals.resize(averageNormals.size());

    //resolve current weights and indices
    int weightSetLength = baseVertices.size() * weightsPerVertex;
    float* currentWeights = &vertexWeights[activeSet * weightSetLength];
    int* currentIndices = &weightIndices[(activeSet / 2) * weightSetLength];

    for (int i = 0;i < baseTangents.size();i++) {
        skinnedTangents[i] = glm::vec3(0.0f);
        skinnedAverageNormals[i] = glm::vec3(.0f);
        for (int j = 0;j < weightsPerVertex;j++) {
            glm::vec3 tangentContribution = currentFrame[currentIndices[i *
                weightsPerVertex + j]].rotscale * baseTangents[i];
            glm::vec3 normalContribution = currentFrame[currentIndices[i *
                weightsPerVertex + j]].rotscale * averageNormals[i];

            skinnedTangents[i] += currentWeights[i * weightsPerVertex + j] * tangentContribution;
            skinnedAverageNormals[i] += currentWeights[i * weightsPerVertex + j] * normalContribution;
        }
        skinnedTangents[i] = glm::normalize(skinnedTangents[i]);
        skinnedAverageNormals[i] = glm::normalize(skinnedAverageNormals[i]);
    }



    std::vector<glm::vec3> smoothedVertices;
    smoothedVertices.resize(baseVertices.size());
    smoothLaplacian(positions, &smoothedVertices[0], baseVertices.size(), adjacency);
    std::vector<glm::vec3> smoothedNormals;
    smoothedNormals.resize(baseNormals.size());
    smoothLaplacian(&skinnedAverageNormals[0], &smoothedNormals[0], baseNormals.size(), adjacency);
    std::vector<glm::vec3> smoothedTangents;
    smoothedTangents.resize(baseTangents.size());
    smoothLaplacian(&skinnedTangents[0], &smoothedTangents[0], baseTangents.size(), adjacency);
    for (int i = 0;i < baseVertices.size();i++) {
        //compute bitangent as cross product of normal and tangent
        glm::vec3 bitangent = glm::cross(glm::normalize(smoothedNormals[i]), glm::normalize(smoothedTangents[i]));
        //coordinate transition matrix
        glm::mat3 coord(glm::normalize(smoothedTangents[i]), glm::normalize(smoothedNormals[i]), glm::normalize(bitangent));


        positions[i] = smoothedVertices[i] + coord * deltas[i];
    }
}

void AnimationData::applyDeltaMush(glm::vec3* positions, glm::vec3* normals, float frame, const AnimationClip& clip, VertexWeightSet activeSet) {
    float absoluteFrame = clip.offset + fmodf(frame, clip.length);
    applyDeltaMush(positions, normals, absoluteFrame, activeSet);
}