#include <iostream>
#include <glm/vec3.hpp>
#include "modelloader.cpp"
#include "animation.h"
#include "camera.hpp"

#ifdef _WIN32
#include<libloaderapi.h>
#include<direct.h>
#else
#include<unistd.h>
#endif

//simple macro to copy function name as string literal
#define TEST(f) f,#f

int totalTests;
int successfulTests;

void runTest(bool(*test)(void), const char* testName) {
    std::cout << "Running " << testName << "... ";
    totalTests++;
    successfulTests += test();
}

bool testVertexData() {
    positionBuffer.clear();
    normalBuffer.clear();
    indexBuffer.clear();
    Model m = loadIQM("./assets/testcube.iqm");
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
    std::cout << "TODO\n";
    return true;
}

bool expectValue(const glm::vec3& actual, const glm::vec3& expected, const char* name) {
    if (fabs(glm::length(actual - expected)) > std::numeric_limits<float>::epsilon()) {
        std::cout << "FAILED\n \tExpected " << name << " value (" << expected.x << "," << expected.y << "," << expected.z << ")\n";
        std::cout << "\tGot: (" << actual.x << "," << actual.y << "," << actual.z << ")\n";
        return false;
    }
    return true;
}
bool expectValue(const float actual, float expected, const char* name) {
    if (fabs(actual - expected) > std::numeric_limits<float>::epsilon()) {
        std::cout << "FAILED\n \tExpected " << name << " value " << expected << "\n";
        std::cout << "\tGot: " << actual << "\n";
        return false;
    }
    return true;
}

bool testArcballNull() {
    const glm::vec3 startingEye(.0f, .0f, 1.0f);
    const glm::vec3 startingCenter(.0f);
    const glm::vec3 startingUp(.0f, 1.0f, .0f);
    Camera camera(startingEye, startingCenter, startingUp);
    camera.rotateArcball(glm::vec2(.0f), glm::vec2(.0f));

    if (!expectValue(camera.getEye(), startingEye, "eye"))
        return false;
    if (!expectValue(camera.getCenter(), startingCenter, "center"))
        return false;
    if (!expectValue(camera.getUp(), startingUp, "up"))
        return false;
    std::cout << "PASSED\n";
    return true;
}

bool testArcballEye() {
    const glm::vec3 startingEye(.0f, .0f, 1.0f);
    const glm::vec3 startingCenter(.0f);
    const glm::vec3 startingUp(.0f, 1.0f, .0f);
    Camera camera(startingEye, startingCenter, startingUp);

    camera.rotateArcball(glm::vec2(.5f), glm::vec2(.75f, .5f));

    const glm::vec3 finalEye(-sin(glm::radians(60.0f)), .0f, cos(glm::radians(60.0f)));
    if (!expectValue(camera.getEye(), finalEye, "eye"))
        return false;

    std::cout << "PASSED\n";
    return true;
}

bool testArcballEye2() {
    const glm::vec3 startingEye(.0f, .0f, 1.0f);
    const glm::vec3 startingCenter(.0f);
    const glm::vec3 startingUp(.0f, 1.0f, .0f);
    Camera camera(startingEye, startingCenter, startingUp);

    camera.rotateArcball(glm::vec2(.5f), glm::vec2(.5f, .75f));

    const glm::vec3 finalEye(.0f, sin(glm::radians(60.0f)), cos(glm::radians(60.0f)));
    if (!expectValue(camera.getEye(), finalEye, "eye"))
        return false;
    std::cout << "PASSED\n";
    return true;
}

bool testArcballUp() {
    const glm::vec3 startingEye(.0f, .0f, 1.0f);
    const glm::vec3 startingCenter(.0f);
    const glm::vec3 startingUp(.0f, 1.0f, .0f);
    Camera camera(startingEye, startingCenter, startingUp);

    camera.rotateArcball(glm::vec2(.5f), glm::vec2(1.0f, .5f));

    //rotating around y axis - should be unchanged
    if (!expectValue(camera.getUp(), startingUp, "up"))
        return false;
    std::cout << "PASSED\n";
    return true;
}

bool testArcballUp2() {
    const glm::vec3 startingEye(.0f, .0f, 1.0f);
    const glm::vec3 startingCenter(.0f);
    const glm::vec3 startingUp(.0f, 1.0f, .0f);
    Camera camera(startingEye, startingCenter, startingUp);

    camera.rotateArcball(glm::vec2(.5f), glm::vec2(.5f, .75f));

    const glm::vec3 finalUp(.0f, cos(glm::radians(60.0f)), -sin(glm::radians(60.0f)));
    if (!expectValue(camera.getUp(), finalUp, "up"))
        return false;
    std::cout << "PASSED\n";
    return true;
}

bool testArcballSequential() {
    const glm::vec3 startingEye(.0f, .0f, 1.0f);
    const glm::vec3 startingCenter(.0f);
    const glm::vec3 startingUp(.0f, 1.0f, .0f);
    Camera camera(startingEye, startingCenter, startingUp);

    camera.rotateArcball(glm::vec2(.0f, .0f), glm::vec2(.5f, .0f));
    camera.rotateArcball(glm::vec2(.5f, .0f), glm::vec2(.5f, .5f));
    camera.rotateArcball(glm::vec2(.5f, .5f), glm::vec2(.0f, .5f));
    camera.rotateArcball(glm::vec2(.0f, .5f), glm::vec2(.0f, .0f));
    if (!expectValue(camera.getUp(), startingUp, "up"))
        return false;
    std::cout << "PASSED\n";
    return true;
}

bool testPan() {
    const glm::vec3 startingEye(.0f, .0f, 1.0f);
    const glm::vec3 startingCenter(.0f);
    const glm::vec3 startingUp(.0f, 1.0f, .0f);
    Camera camera(startingEye, startingCenter, startingUp);

    camera.pan(glm::vec2(.5f, .0f));
    if (!expectValue(camera.getCenter(), glm::vec3(1.0f, .0f, .0f), "center"))
        return false;

    if (!expectValue(camera.getEye(), glm::vec3(1.0f, .0f, 1.0f), "eye"))
        return false;

    std::cout << "PASSED\n";
    return true;
}

bool testPan2() {
    const glm::vec3 startingEye(.0f, .0f, 1.0f);
    const glm::vec3 startingCenter(.0f);
    const glm::vec3 startingUp(.0f, 1.0f, .0f);
    Camera camera(startingEye, startingCenter, startingUp);

    camera.pan(glm::vec2(.0f, .5f));
    if (!expectValue(camera.getCenter(), glm::vec3(.0f, 1.0f, .0f), "center"))
        return false;

    if (!expectValue(camera.getEye(), glm::vec3(.0f, 1.0f, 1.0f), "eye"))
        return false;

    std::cout << "PASSED\n";
    return true;
}

bool testPan3() {
    const glm::vec3 startingEye(.0f, .0f, 1.0f);
    const glm::vec3 startingCenter(.0f);
    const glm::vec3 startingUp(.0f, 1.0f, .0f);
    Camera camera(startingEye, startingCenter, startingUp);

    camera.pan(glm::vec2(.5f, .5f));
    if (!expectValue(camera.getCenter(), glm::vec3(1.0f, 1.0f, .0f), "center"))
        return false;

    if (!expectValue(camera.getEye(), glm::vec3(1.0f, 1.0f, 1.0f), "eye"))
        return false;

    std::cout << "PASSED\n";
    return true;
}


bool testZoom() {
    const glm::vec3 startingEye(.0f, .0f, 1.0f);
    const glm::vec3 startingCenter(.0f);
    const glm::vec3 startingUp(.0f, 1.0f, .0f);
    Camera camera(startingEye, startingCenter, startingUp);

    camera.zoom(.5f);
    if (!expectValue(camera.getEye(), glm::vec3(.0f, .0f, .5f), "eye"))
        return false;

    std::cout << "PASSED\n";
    return true;
}

bool testZoomClamp() {
    const glm::vec3 startingEye(.0f, .0f, 1.0f);
    const glm::vec3 startingCenter(.0f);
    const glm::vec3 startingUp(.0f, 1.0f, .0f);
    Camera camera(startingEye, startingCenter, startingUp);

    camera.zoom(2.0f);
    if (!expectValue(camera.cameraZoom, .0001f, "zoom"))
        return false;

    std::cout << "PASSED\n";
    return true;
}

bool testAdjacencyList() {
    unsigned int faces[6] = {
        0,1,2,
        3,4,5,
    };

    int adjacency[6][2] = {
        {1,2},
        {0,2},
        {0,1},
        {4,5},
        {3,5},
        {3,4}
    };

    std::vector<std::vector<int> > target;
    generateAdjacencyList(target, faces, 2, 6);


    if (target.size() != 6) {
        std::cout << "FAILED: expected list size 6, got: " << target.size() << "\n";
        return false;
    }
    for (int i = 0;i < 6;i++) {
        if (target[i].size() != 2) {
            std::cout << "FAILED: expected 2 neighbours, got: " << target[i].size() << "\n";
            return false;
        }
        for (int j = 0;j < 2;j++) {
            //check that all neighbours are found
            bool found = false;
            for (int x = 0;x < 2;x++) {
                if (target[i][x] == adjacency[i][j]) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                std::cout << "FAILED: Neighbour " << adjacency[i][j] << "not found\n";
                return false;
            }
        }
    }
    std::cout << "PASSED\n";
    return true;
}

bool testLaplacianSmooth() {
    glm::vec3 positions[4] = {
        glm::vec3(-1.0f,.0f,-1.0f),
        glm::vec3(1.0f,.0f,-1.0f),
        glm::vec3(.0f,.0f,1.0f),
        glm::vec3(.0f,1.0f,.0f),
    };
    glm::vec3 newPositions[4];

    std::vector<std::vector<int> > adjacency;
    adjacency.push_back(std::vector<int>());
    adjacency[0].push_back(1);
    adjacency[0].push_back(2);
    adjacency[0].push_back(3);
    adjacency.push_back(std::vector<int>());
    adjacency[1].push_back(0);
    adjacency[1].push_back(2);
    adjacency[1].push_back(3);
    adjacency.push_back(std::vector<int>());
    adjacency[2].push_back(0);
    adjacency[2].push_back(1);
    adjacency[2].push_back(3);
    adjacency.push_back(std::vector<int>());
    adjacency[3].push_back(0);
    adjacency[3].push_back(1);
    adjacency[3].push_back(2);

    smoothLaplacian(positions, newPositions, 4, adjacency, 1);

    const glm::vec3 expectedPositions[4] = {
        glm::vec3(1.0f / 3.0f,1.0f / 3.0f,.0f),
        glm::vec3(-1.0f / 3.0f,1.0f / 3.0f,.0f),
        glm::vec3(.0f,1.0f / 3.0f,-2.0f / 3.0f),
        glm::vec3(.0f,.0f,-1.0f / 3.0f),
    };

    for (int i = 0;i < 4;i++) {
        if (!expectValue(newPositions[i], expectedPositions[i], "smoothed vertex"))
            return false;
    }

    std::cout << "PASSED\n";
    return true;

}

int main() {
    {
        char executablePath[MAX_PATH];
#ifdef _WIN32
        int pathLen = GetModuleFileName(NULL, executablePath, MAX_PATH);
#else
        int pathLen = readlink("/proc/self/exe", executablePath, MAX_PATH);
#endif
        //remove executable name
        for (pathLen--;pathLen >= 0 && executablePath[pathLen] != '\\';pathLen--) {
            executablePath[pathLen] = '\0';
        }

        chdir(executablePath);
    }


    totalTests = 0;
    successfulTests = 0;

    runTest(TEST(testVertexData));
    runTest(TEST(testArcballNull));
    runTest(TEST(testArcballEye));
    runTest(TEST(testArcballEye2));
    runTest(TEST(testArcballUp));
    runTest(TEST(testArcballUp2));
    runTest(TEST(testPan));
    runTest(TEST(testPan2));
    runTest(TEST(testPan3));
    runTest(TEST(testZoom));
    runTest(TEST(testZoomClamp));
    runTest(TEST(testArcballSequential));
    runTest(TEST(testAdjacencyList));
    runTest(TEST(testLaplacianSmooth));

    std::cout << "Tests Passed: " << successfulTests << "/" << totalTests;
    return 0;
}
