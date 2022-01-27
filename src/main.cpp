#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include<iostream>

#include "modelloader.hpp"

int main() {
    //setup window
    GLFWwindow *window;
    glfwInit();

    glfwWindowHint(GLFW_MAXIMIZED, 1);
    glfwWindowHint(GLFW_SAMPLES, 4);

    window = glfwCreateWindow(1600, 900, "Skinning Dissertation", NULL, NULL);

    glfwMakeContextCurrent(window);
    glClearColor(.7f, .7f, .7f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glfwSwapBuffers(window);
    glfwPollEvents();

    //setup OpenGL
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cout << "Error: " << glewGetErrorString(err) << "\n";
        return 1;
    }


    graphicsInit();
    //load models
    Model longboi = loadIQM(ROOTDIR "/assets/longboi(vibe).iqm");
    uploadBuffers();



    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        longboi.draw();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    return 0;
}