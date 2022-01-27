#include <iostream>
#include <glm/vec3.hpp>
#include "modelloader.cpp"

int totalTests;
int successfulTests;

void runTest(bool(*test)(void)) {
    totalTests++;
    successfulTests += test();
}

bool testVertexData() {
    std::cout << "Testing vertex loading... ";
    positionBuffer.clear();
    normalBuffer.clear();
    indexBuffer.clear();
    Model m = loadIQM(ROOTDIR "/assets/testcube.iqm");
    glm::vec3 positions[8] = {
        glm::vec3(1.0f,1.0f,1.0f),
        glm::vec3(1.0f,-1.0f,1.0f),
        glm::vec3(1.0f,1.0f,-1.0f),
        glm::vec3(1.0f,-1.0f,-1.0f),
        glm::vec3(-1.0f,1.0f,1.0f),
        glm::vec3(-1.0f,-1.0f,1.0f),
        glm::vec3(-1.0f,1.0f,-1.0f),
        glm::vec3(-1.0f,-1.0f,-1.0f),
    };
    glm::vec3 normals[24] = {
        glm::vec3(1.0f,.0f,.0f),
        glm::vec3(.0f,1.0f,.0f),
        glm::vec3(.0f,.0f,1.0f),
        glm::vec3(1.0f,.0f,.0f),
        glm::vec3(.0f,-1.0f,.0f),
        glm::vec3(.0f,.0f,1.0f),
        glm::vec3(1.0f,.0f,.0f),
        glm::vec3(.0f,1.0f,.0f),
        glm::vec3(.0f,.0f,-1.0f),
        glm::vec3(1.0f,.0f,.0f),
        glm::vec3(.0f,-1.0f,.0f),
        glm::vec3(.0f,.0f,-1.0f),
        glm::vec3(-1.0f,.0f,.0f),
        glm::vec3(.0f,1.0f,.0f),
        glm::vec3(.0f,.0f,1.0f),
        glm::vec3(-1.0f,.0f,.0f),
        glm::vec3(.0f,-1.0f,.0f),
        glm::vec3(.0f,.0f,1.0f),
        glm::vec3(-1.0f,.0f,.0f),
        glm::vec3(.0f,1.0f,.0f),
        glm::vec3(.0f,.0f,-1.0f),
        glm::vec3(-1.0f,.0f,.0f),
        glm::vec3(.0f,-1.0f,.0f),
        glm::vec3(.0f,.0f,-1.0f),
    };
    bool positionsFound[8] = { false };
    bool normalsFound[24] = { false };


    for (int i = 0;i < positionBuffer.size();i++) {
        //find position
        int posIdx = -1;
        for (int j = 0;j < 8;j++) {
            if (positions[j] == positionBuffer[i]) {
                posIdx = j;
                positionsFound[j] = true;
                break;
            }
        }
        if (posIdx == -1) {
            std::cout << "FAILED: Incorrect position (" << positionBuffer[i].x << ", "
                << positionBuffer[i].y << ", " << positionBuffer[i].z << ")\n";
            return false;
        }

        //check normals
        if (normals[3 * posIdx] == normalBuffer[i]) {
            normalsFound[3 * posIdx] = true;
        }
        else if (normals[3 * posIdx + 1] == normalBuffer[i]) {
            normalsFound[3 * posIdx + 1] = true;
        }
        else if (normals[3 * posIdx + 2] == normalBuffer[i]) {
            normalsFound[3 * posIdx + 2] = true;
        }
        else {
            std::cout << "FAILED: Incorrect normal (" << positionBuffer[i].x << ", "
                << positionBuffer[i].y << ", " << positionBuffer[i].z << ")\n";
            return false;
        }

    }

    //check if all positions are loaded
    for (int i = 0;i < 8;i++) {
        if (!positionsFound[i]) {
            std::cout << "FAILED: Missing position (" << positions[i].x << ", "
                << positions[i].y << ", " << positions[i].z << ")\n";
            return false;
        }
    }
    //check if all normals are loaded
    for (int i = 0;i < 24;i++) {
        if (!normalsFound[i]) {
            std::cout << "FAILED: Missing normal (" << normals[i].x << ", "
                << normals[i].y << ", " << normals[i].z << ") for vertex " << positions[i / 3].x << ", "
                << positions[i / 3].y << ", " << positions[i / 3].z << ")\n";
            return false;
        }
    }
    std::cout << "PASSED\n";
    return true;
}

bool testCubeDiff() {
    std::cout << "Testing cube diff: TODO";
    return true;
}


int main() {
    totalTests = 0;
    successfulTests = 0;

    runTest(testVertexData);


    std::cout << "Tests Passed: " << successfulTests << "/" << totalTests;
    return 0;
}