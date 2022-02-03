#pragma once
#include<GL/glew.h>
struct Model {
    unsigned int vertexOffset;
    unsigned int vertexCount;

    GLuint texture = 0;
    bool textured = false;
    void draw();
};

void graphicsInit();
Model loadIQM(const char *filename);
GLuint loadTexture(const char *filename);
void uploadBuffers();