#include "animation.h"
#include <GL/glew.h>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

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
            glm::vec4 jointCent=joints[i].matrix * glm::vec4(.0f, .0f, .0f, 1.0f);
            jointCenters[i] = glm::vec3(jointCent.x,jointCent.y,jointCent.z);
            //touch for the sake of feng compatibility
            volatile float touch=jointCenters[i].x+jointCenters[i].y+jointCenters[i].z;
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
            int minInd=0;
            if (startInd < posesPerFrame){
                glm::vec3 minVec=jointCenters[startInd] - baseVertices[i];
                minDist = minVec.x*minVec.x+minVec.y*minVec.y+minVec.z*minVec.z;
                minInd=startInd;
            }

            for (int j = startInd;j < posesPerFrame;j++) {
                glm::vec3 newVec=(glm::vec3)jointCenters[j] - baseVertices[i];
                float newDist = newVec.x*newVec.x+newVec.y*newVec.y+newVec.z*newVec.z;
                if ((newDist < minDist) && isJointActive[j]) {
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
            weightIndices.push_back(minInd);
            vertexWeights.push_back(1.0f);
            for (int j = 1;j < weightsPerVertex;j++) {
                weightIndices.push_back(0);
                vertexWeights.push_back(0.0f);
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
    short indexSet=activeSet/2;
    int* currentIndices = &weightIndices[indexSet * weightSetLength];

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
    surfaceNormals.clear();
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
void generateAdjacencyList(std::vector<std::vector<int> >& target, const unsigned int* faces, int faceCount, int vertexCount) {
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

void smoothLaplacian(glm::vec3* source, glm::vec3* target, int count, std::vector<std::vector<int> >& adjacency, int iterations) {
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

static const int DEFAULTLAPLACIANITERATIONS = 10;


static void computeSurfaceNormals(glm::vec3* target, glm::vec3* vertices, size_t vertexCount, unsigned int* faces,
    int faceCount, std::vector<std::vector<int> >& duplicates) {
    //initialise per-vertex sums
    for (int i = 0;i < vertexCount;i++) {
        target[i] = glm::vec3(.0f);
    }


    //compute a normal for each face
    for (int i = 0;i < faceCount;i++) {
        glm::vec3 faceNormal = glm::normalize(glm::cross(vertices[faces[3 * i]] - vertices[faces[3 * i + 2]],
            vertices[faces[3 * i + 1]] - vertices[faces[3 * i + 2]]));
        target[faces[3 * i]] += faceNormal;
        target[faces[3 * i + 1]] += faceNormal;
        target[faces[3 * i + 2]] += faceNormal;
    }

    //we can have multiple copies of each position as part of different faces
    //should all have the same surface normal

    for (int i = 0;i < duplicates.size();i++) {
        glm::vec3 combinedSum = glm::vec3(.0f);

        //computed combined sum
        for (int j = 0;j < duplicates[i].size();j++) {
            combinedSum += target[duplicates[i][j]];
        }

        for (int j = 0;j < duplicates[i].size();j++) {
            target[duplicates[i][j]] = glm::normalize(combinedSum);
        }
    }
}

void AnimationData::prepareDeltaMush(unsigned int* faces, int faceCount) {
    //find duplicate vertices
    {
        duplicates.clear();
        std::vector<bool> consolidated(baseVertices.size(), false);

        for (int i = 0;i < baseVertices.size();i++) {
            if (!consolidated[i]) {
                std::vector<int> duplicateIndices;
                //find all vertices with identical positions
                for (int j = i;j < baseVertices.size();j++) {
                    if (baseVertices[i] == baseVertices[j]) {
                        duplicateIndices.push_back(j);
                        consolidated[j] = true;
                    }
                }
                duplicates.push_back(duplicateIndices);
            }
        }
    }
    //prepare vertex data for smoothing
    {
        //generates simple adjacency only, doesn't compute duplicates
        generateAdjacencyList(adjacency, faces, faceCount, baseVertices.size());

        //we can have multiple copies of each position with different normals/tangents
        //need to fix adjacency
        for (int i = 0;i < duplicates.size();i++) {
            //propagate averages and consolidate adjacency lists
            std::vector<int> combinedAdjacencies;
            for (int j = 0;j < duplicates[i].size();j++) {
                for (int k = 0;k < adjacency[duplicates[i][j]].size();k++) {
                    bool found = false;
                    for (int l = 0;l < combinedAdjacencies.size();l++) {
                        if (baseVertices[combinedAdjacencies[l]] == baseVertices[adjacency[duplicates[i][j]][k]])
                            found = true;
                        break;
                    }
                    if (!found)
                        combinedAdjacencies.push_back(adjacency[duplicates[i][j]][k]);
                }
            }
            for (int j = 0;j < duplicates[i].size();j++) {
                adjacency[duplicates[i][j]].resize(combinedAdjacencies.size());
                for (int k = 0;k < combinedAdjacencies.size();k++) {
                    adjacency[duplicates[i][j]][k] = combinedAdjacencies[k];
                }
            }
        }
    }

    //solve rest pose for offsets
    {
        //compute a smoothed set of vertices
        std::vector<glm::vec3> smoothedVertices;
        smoothedVertices.resize(baseVertices.size());
        smoothLaplacian(&baseVertices[0], &smoothedVertices[0], baseVertices.size(), adjacency, DEFAULTLAPLACIANITERATIONS);

        //find the corresponding surface normal
        surfaceNormals.resize(baseNormals.size());

        computeSurfaceNormals(&surfaceNormals[0], &smoothedVertices[0], smoothedVertices.size(),
            faces, faceCount, duplicates);

        //calculate deltas
        deltas.resize(baseVertices.size());
        for (int i = 0;i < baseVertices.size();i++) {
            //create local coordinate frame
            glm::vec3 normal = surfaceNormals[i];
            glm::vec3 tangent = glm::normalize(glm::cross(glm::normalize(smoothedVertices[adjacency[i][0]]
                - smoothedVertices[i]), normal));
            glm::vec3 bitangent = glm::normalize(glm::cross(normal, tangent));

            //get offset in terms of basis
            glm::mat3 restCoordinates = glm::mat3(tangent, normal, bitangent);

            deltas[i] = glm::inverse(restCoordinates) * (baseVertices[i] - smoothedVertices[i]);
        }
    }
    deltaMushReady = true;
}

void AnimationData::applyDeltaMush(glm::vec3* positions, unsigned int* faces, int faceCount) {

    //perform Laplacian smoothing on all required components
    std::vector<glm::vec3> smoothedVertices;
    smoothedVertices.resize(baseVertices.size());
    smoothLaplacian(positions, &smoothedVertices[0], baseVertices.size(), adjacency, DEFAULTLAPLACIANITERATIONS);

    computeSurfaceNormals(&surfaceNormals[0], &smoothedVertices[0], smoothedVertices.size(),
        faces, faceCount, duplicates);

    for (int i = 0;i < baseVertices.size();i++) {
        //find new basis
        glm::vec3 normal = surfaceNormals[i];
        glm::vec3 tangent = glm::normalize(glm::cross(glm::normalize(smoothedVertices[adjacency[i][0]]
            - smoothedVertices[i]), normal));
        glm::vec3 bitangent = glm::normalize(glm::cross(normal, tangent));

        //get offset in terms of bases
        glm::mat3 coord = glm::mat3(tangent, normal, bitangent);

        positions[i] = smoothedVertices[i] + coord * deltas[i];
    }
}
