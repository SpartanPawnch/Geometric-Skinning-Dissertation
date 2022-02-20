#include <iostream>
#include <glm/vec3.hpp>
#include "modelloader.cpp"
#include "camera.hpp"

//simple macro to copy function name as string literal
#define TEST(f) f,#f

int totalTests;
int successfulTests;

void runTest(bool(*test)(void),const char *testName) {
    std::cout<<"Running "<<testName<<"... ";
    totalTests++;
    successfulTests += test();
}

bool testVertexData() {
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
    std::cout << "TODO\n";
    return true;
}

bool expectValue(const glm::vec3 &actual, const glm::vec3 &expected, const char* name){
    if(actual!=expected){
        std::cout<<"FAILED\n \tExpected "<<name<<" value ("<<expected.x<<","<<expected.y<<","<<expected.z<<")\n";
        std::cout<<"\tGot: ("<<actual.x<<","<<actual.y<<","<<actual.z<<")\n";
        return false;
    }
    return true;
}

bool testArcballNull(){
    Camera camera;
    const glm::vec3 startingEye(.0f,.0f,1.0f);
    const glm::vec3 startingCenter(.0f);
    const glm::vec3 startingUp(.0f,1.0f,.0f);
    camera.eye=startingEye;
    camera.center=startingCenter;
    camera.up=startingUp;
    camera.rotateArcball(glm::vec2(.0f),glm::vec2(.0f));

    if(!expectValue(camera.eye,startingEye,"eye"))
        return false;
    if(!expectValue(camera.center,startingCenter,"center"))
        return false;
    if(!expectValue(camera.up,startingUp,"up"))
        return false;
    std::cout<< "PASSED\n";
    return true;
}

bool testArcballEye(){
    Camera camera;
    const glm::vec3 startingEye(.0f,.0f,1.0f);
    const glm::vec3 startingCenter(.0f);
    const glm::vec3 startingUp(.0f,1.0f,.0f);
    camera.eye=startingEye;
    camera.center=startingCenter;
    camera.up=startingUp;
    camera.rotateArcball(glm::vec2(.0f),glm::vec2(.5f,.0f));

    const glm::vec3 finalEye(sin(glm::radians(60.0f)),.0f,cos(glm::radians(60.0f)));
    if(!expectValue(camera.eye,finalEye,"eye"))
        return false;

    std::cout<<"PASSED\n";
    return true;
}

bool testArcballEye2(){
    Camera camera;
    const glm::vec3 startingEye(.0f,.0f,1.0f);
    const glm::vec3 startingCenter(.0f);
    const glm::vec3 startingUp(.0f,1.0f,.0f);
    camera.eye=startingEye;
    camera.center=startingCenter;
    camera.up=startingUp;
    camera.rotateArcball(glm::vec2(.0f),glm::vec2(.0f,.5f));

    const glm::vec3 finalEye(.0f,sin(glm::radians(60.0f)),cos(glm::radians(60.0f)));
    if(!expectValue(camera.eye,finalEye,"eye"))
        return false;
    std::cout<<"PASSED\n";
    return true;
}

bool testArcballUp(){
    Camera camera;
    const glm::vec3 startingEye(.0f,.0f,1.0f);
    const glm::vec3 startingCenter(.0f);
    const glm::vec3 startingUp(.0f,1.0f,.0f);
    camera.eye=startingEye;
    camera.center=startingCenter;
    camera.up=startingUp;
    camera.rotateArcball(glm::vec2(.0f),glm::vec2(.5f,.0f));

    //rotating around y axis - should be unchanged
    if(!expectValue(camera.up,startingUp,"up"))
        return false;
    std::cout<<"PASSED\n";
    return true;
}

bool testArcballUp2(){
    Camera camera;
    const glm::vec3 startingEye(.0f,.0f,1.0f);
    const glm::vec3 startingCenter(.0f);
    const glm::vec3 startingUp(.0f,1.0f,.0f);
    camera.eye=startingEye;
    camera.center=startingCenter;
    camera.up=startingUp;
    camera.rotateArcball(glm::vec2(.0f),glm::vec2(.0f,.5f));

    const glm::vec3 finalUp(.0f,cos(glm::radians(60.0f)),sin(glm::radians(60.0f)));
    if(!expectValue(camera.up,finalUp,"up"))
        return false;
    std::cout<<"PASSED\n";
    return true;
}

bool testPan(){
    std::cout<<"TODO\n";
    return true;
}

bool testZoom(){
    std::cout<<"TODO\n";
    return true;
}

int main() {
    totalTests = 0;
    successfulTests = 0;

    runTest(TEST(testVertexData));
    runTest(TEST(testArcballNull));
    runTest(TEST(testArcballEye));
    runTest(TEST(testArcballEye2));
    runTest(TEST(testArcballUp));
    runTest(TEST(testArcballUp2));
    runTest(TEST(testPan));
    runTest(TEST(testZoom));


    std::cout << "Tests Passed: " << successfulTests << "/" << totalTests;
    return 0;
}