#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include<iostream>

#include "modelloader.hpp"

int main() {
    //setup window
    GLFWwindow *window;
    glfwInit();

    #if GLFW_VERSION_MAJOR>=3 && GLFW_VERSION_MINOR>=1
    //Supported by glfw 3.1 and onwards
    glfwWindowHint(GLFW_MAXIMIZED, 1);
    #endif

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
    Model longboi = loadIQM(ROOTDIR "/assets/longboi.iqm");
    longboi.texture = loadTexture(ROOTDIR "/assets/longboi_texture.png");
    longboi.textured = true;
    uploadBuffers();

    float animtime=.0f,lasttime=glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        float frametime=glfwGetTime()-lasttime;
        lasttime+=frametime;
        animtime+=60*frametime;

        while(animtime>=(float)longboi.animationData.poses.size()/longboi.animationData.posesPerFrame){
            animtime-=longboi.animationData.poses.size()/longboi.animationData.posesPerFrame;
        }
        longboi.animate(animtime);

        int width,height;
        glfwGetFramebufferSize(window,&width,&height);
        setAspectRatio((float)width/height);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        longboi.draw();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    return 0;
}
