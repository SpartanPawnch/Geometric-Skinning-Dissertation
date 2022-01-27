#pragma once
struct Model {
    unsigned int vertexOffset;
    unsigned int vertexCount;
    void draw();
};

void graphicsInit();
Model loadIQM(const char *filename);
void uploadBuffers();