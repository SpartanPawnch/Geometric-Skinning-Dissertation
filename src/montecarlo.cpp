#include<iostream>
#include<random>
#include<ctime>
#include<cmath>

#include<glm/gtx/transform.hpp>
#include<glm/vec3.hpp>

#ifdef _WIN32
#include<libloaderapi.h>
#include<direct.h>
#else
#include<unistd.h>
#endif

#include "modelloader.cpp"
//check if ray parralel to positive z axis from point intersects with triangle abc
bool checkIntersection(glm::vec3 point, glm::vec3 a, glm::vec3 b, glm::vec3 c) {
    //find barycentric coordinates
    float alpha, beta;
    if (a.y - c.y >= std::numeric_limits<float>::epsilon() ||
        a.y - c.y <= -std::numeric_limits<float>::epsilon()) {
        beta = (point.x - c.x - (point.y - c.y) * (a.x - c.x) / (a.y - c.y)) /
            (b.x - c.x - (a.x - c.x) * (b.y - c.y) / (a.y - c.y));
        alpha = (point.y - c.y - beta * (b.y - c.y)) / (a.y - c.y);
    }
    else if (b.y - c.y >= std::numeric_limits<float>::epsilon() ||
        b.y - c.y <= -std::numeric_limits<float>::epsilon()) {
        beta = (point.y - c.y) / (b.y - c.y);
        alpha = ((point.x - c.x) - beta * (b.x - c.x)) / (a.x - c.x);
    }
    else {
        return (point.y == c.y);
    }

    float z = alpha * a.z + beta * b.z + (1 - alpha - beta) * c.z;

    return((z >= point.z) && (alpha >= .0f) && (alpha <= 1.0f) && (beta >= .0f) &&
        (beta <= 1.0f) && (alpha + beta >= .0f) && (alpha + beta <= 1.0f));
}

bool checkInside(glm::vec3 point, const glm::vec3* vertices, const unsigned int* faces, int faceCount) {
    int intersections = 0;
    for (int i = 0;i < faceCount;i++) {
        intersections += checkIntersection(point, vertices[faces[3 * i]], vertices[faces[3 * i + 1]], vertices[faces[3 * i + 2]]);
    }
    return(intersections % 2 == 1);
}

float monteVolume(const glm::vec3* vertices, int vertCount, const unsigned int* faces, int faceCount, int iterations) {
    glm::vec3 maxCorner;
    glm::vec3 minCorner;

    minCorner = vertices[0];
    maxCorner = vertices[0];

    //find bounding box
    for (int i = 0;i < vertCount;i++) {
        if (vertices[i].x > maxCorner.x) {
            maxCorner.x = vertices[i].x;
        }
        if (vertices[i].x < minCorner.x) {
            minCorner.x = vertices[i].x;
        }
        if (vertices[i].y > maxCorner.y) {
            maxCorner.y = vertices[i].y;
        }
        if (vertices[i].y < minCorner.y) {
            minCorner.y = vertices[i].y;
        }
        if (vertices[i].z > maxCorner.z) {
            maxCorner.z = vertices[i].z;
        }
        if (vertices[i].z < minCorner.z) {
            minCorner.z = vertices[i].z;
        }
    }

    std::default_random_engine generator;
    generator.seed(time(NULL));
    std::uniform_real_distribution<float> distribution(0.0, 1.0);

    //test random points
    int insideCount = 0;
    for (int i = 0;i < iterations;i++) {
        //generate random point
        glm::vec3 point;
        float t = distribution(generator);
        point.x = minCorner.x * t + (1 - t) * maxCorner.x;
        t = distribution(generator);
        point.y = minCorner.y * t + (1 - t) * maxCorner.y;
        t = distribution(generator);
        point.z = minCorner.z * t + (1 - t) * maxCorner.z;

        //test if it's inside
        insideCount += checkInside(point, vertices, faces, faceCount);
    }
    return(insideCount * (maxCorner.x - minCorner.x) * (maxCorner.y - minCorner.y) * (maxCorner.z - minCorner.z) / iterations);
}

#ifndef MONTE_TESTING
int main(int argc, char** argv) {

    {
#ifdef _WIN32
        char executablePath[MAX_PATH];
        int pathLen = GetModuleFileName(NULL, executablePath, MAX_PATH);
#else
        char executablePath[PATH_MAX];
        int pathLen = readlink("/proc/self/exe", executablePath, PATH_MAX);
#endif
        //remove executable name
        for (pathLen--;pathLen >= 0 && executablePath[pathLen] != '\\';pathLen--) {
            executablePath[pathLen] = '\0';
        }

        chdir(executablePath);
    }

#ifdef METHOD_EVAL
    glm::vec3 point = glm::vec3(.1f, .1, 3.0f);
    glm::vec3 verts[4] = { glm::vec3(-1.0f,-1.0f,1.0f),glm::vec3(1.0f,-1.0f,1.0f),
        glm::vec3(.0f,-1.0f,-1.0f),glm::vec3(.0f,1.0f,.0f) };

    int faces[12] = {
        0,1,2,
        0,1,3,
        0,2,3,
        1,2,3
    };


    std::cout << "--- Method Evaluation ---\n";
    float errors[5][10];
    //simple accuracy evaluation
    for (int i = 0;i < 10;i++) {
        int startingIterations = 10000;
        for (int j = 0;j < 5;j++) {
            float result = monteVolume(verts, 4, faces, 4, startingIterations);
            const float actual = 4.0f / 3.0f;
            float relError = abs(actual - result) / actual;

            errors[j][i] = relError;

            startingIterations *= 10;
        }
    }


    //aggregate error info
    int iterationCount = 10000;
    for (int i = 0;i < 5;i++) {
        float mean = .0f, min = errors[i][0], max = errors[i][0];
        for (int j = 0;j < 10;j++) {
            mean += errors[i][j];
            max = (errors[i][j] > max ? errors[i][j] : max);
            min = (errors[i][j] < min ? errors[i][j] : min);
        }
        mean /= 10.0f;

        //calculate standard deviation
        float stdDev = .0f;
        for (int j = 0;j < 10;j++) {
            stdDev += (errors[i][j] - mean) * (errors[i][j] - mean);
        }
        stdDev = sqrt(stdDev / 10);

        std::cout << iterationCount << " iterations error: mean: " << mean
            << ", standard deviation: " << stdDev << ", min: " << min << ", max: " << max << "\n";

        iterationCount *= 10;
    }
#endif

    //measure prism
    Model model = loadIQM("./assets/artefactingprism.iqm");
    model.animationData.prepareDeltaMush(&indexBuffer[model.vertexOffset], model.vertexCount / 3);
    int framecount = 0;
    float minVol[2] = { 1000.0f,1000.0f }, avgVol[2] = { .0f,.0f }, maxVol[2] = { .0f,.0f };
    for (float i = .0f;i < model.clips[1].length;i += 10.0f) {
        model.animationData.deformPositionLBS(&positionBuffer[model.bufferOffset], i, model.clips[1], VertexWeightSetBase);
        float result = monteVolume(&positionBuffer[model.bufferOffset], model.animationData.baseVertices.size(),
            &indexBuffer[model.vertexOffset], model.vertexCount / 3, 100000);
        avgVol[0] += result;
        minVol[0] = (result < minVol[0] ? result : minVol[0]);
        maxVol[0] = (result > maxVol[0] ? result : maxVol[0]);
        model.animationData.applyDeltaMush(&positionBuffer[model.bufferOffset], &indexBuffer[model.vertexOffset], model.vertexCount / 3);
        result = monteVolume(&positionBuffer[model.bufferOffset], model.animationData.baseVertices.size(),
            &indexBuffer[model.vertexOffset], model.vertexCount / 3, 100000);
        avgVol[1] += result;
        minVol[1] = (result < minVol[1] ? result : minVol[1]);
        maxVol[1] = (result > maxVol[1] ? result : maxVol[1]);
        std::cout << "Frame " << i << " done\n";
        framecount++;
    }
    avgVol[0] /= (framecount);
    avgVol[1] /= (framecount);

    std::cout << "---Prism Volume---\n";
    std::cout << "--LBS--\n";
    std::cout << "Minimum Volume: " << minVol[0] << ", Average:" << avgVol[0] << ", Maximum: " << maxVol[0] << "\n";
    std::cout << "--DM--\n";
    std::cout << "Minimum Volume: " << minVol[1] << ", Average:" << avgVol[1] << ", Maximum: " << maxVol[1] << "\n";


    //measure humanoid
    model.clear();
    clearBuffers();

    model = loadIQM("./assets/averagebuild.iqm");
    model.animationData.prepareDeltaMush(&indexBuffer[model.vertexOffset], model.vertexCount / 3);
    framecount = 0;
    minVol[0] = 1000.0f;
    minVol[1] = 1000.0f;
    avgVol[0] = .0f;
    avgVol[1] = .0f;
    maxVol[0] = .0f;
    maxVol[1] = .0f;
    for (float i = .0f;i < model.clips[1].length;i += 10.0f) {
        model.animationData.deformPositionLBS(&positionBuffer[model.bufferOffset], i, model.clips[1], VertexWeightSetBase);
        float result = monteVolume(&positionBuffer[model.bufferOffset], model.animationData.baseVertices.size(),
            &indexBuffer[model.vertexOffset], model.vertexCount / 3, 100000);
        avgVol[0] += result;
        minVol[0] = (result < minVol[0] ? result : minVol[0]);
        maxVol[0] = (result > maxVol[0] ? result : maxVol[0]);
        model.animationData.applyDeltaMush(&positionBuffer[model.bufferOffset], &indexBuffer[model.vertexOffset], model.vertexCount / 3);
        result = monteVolume(&positionBuffer[model.bufferOffset], model.animationData.baseVertices.size(),
            &indexBuffer[model.vertexOffset], model.vertexCount / 3, 100000);
        avgVol[1] += result;
        minVol[1] = (result < minVol[1] ? result : minVol[1]);
        maxVol[1] = (result > maxVol[1] ? result : maxVol[1]);
        std::cout << "Frame " << i << " done\n";
        framecount++;
    }
    avgVol[0] /= (framecount);
    avgVol[1] /= (framecount);

    std::cout << "---Humanoid Volume---\n";
    std::cout << "--LBS--\n";
    std::cout << "Minimum Volume: " << minVol[0] << ", Average:" << avgVol[0] << ", Maximum: " << maxVol[0] << "\n";
    std::cout << "--DM--\n";
    std::cout << "Minimum Volume: " << minVol[1] << ", Average:" << avgVol[1] << ", Maximum: " << maxVol[1] << "\n";

    return 0;
}
#endif