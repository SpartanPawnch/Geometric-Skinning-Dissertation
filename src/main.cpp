#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_opengl2.h>
#include <imgui_impl_glfw.h>

#include <math.h>
#include <iostream>

#define GLM_FORCE_PRECISION_LOWP_FLOAT
#define GLM_FORCE_INLINE

#include "modelloader.hpp"
#include "../external/tinyfiledialogs/tinyfiledialogs.h"

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

    //swap buffers to avoid the window not responding
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

    //setup Imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io=ImGui::GetIO();

    ImGui_ImplGlfw_InitForOpenGL(window,true);
    ImGui_ImplOpenGL2_Init();

    const ImGuiViewport *mainViewport=ImGui::GetMainViewport();

    //load models
    Model activeModel = loadIQM(ROOTDIR "/assets/longboi.iqm");
    activeModel.texture = loadTexture(ROOTDIR "/assets/longboi_texture.png");
    activeModel.textured = true;
    uploadBuffers();
    activeModel.currentClip=1;

    float animtime=.0f,lasttime=glfwGetTime();
    bool paused=false;    
    while (!glfwWindowShouldClose(window)) {
        bool seeking=false;

        //State Processing
        float frametime=glfwGetTime()-lasttime;
        lasttime+=frametime;

        float animationDuration=1.0f,framerate=60.0f;
        if(activeModel.currentClip>=0){
            framerate=activeModel.clips[activeModel.currentClip].framerate;
            animationDuration=activeModel.clips[activeModel.currentClip].length;
        }
        else if(activeModel.animatable){
            animationDuration=((float)activeModel.animationData.poses.size())/activeModel.animationData.posesPerFrame;
        }
        animtime+=!paused*frametime*framerate;
        animtime=fmodf(animtime,animationDuration);
        //get window parameters
        int width,height;
        glfwGetFramebufferSize(window,&width,&height);


        //UI Input
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        {
            //top menubar
            if(ImGui::BeginMainMenuBar()){
                if(ImGui::BeginMenu("File")){
                    if(ImGui::MenuItem("Open Model")){
                        const char* filter="*.iqm";
                        const char* path=tinyfd_openFileDialog(NULL,NULL,1,&filter,NULL,0);
                        if(path!=NULL){
                            activeModel.clear();
                            clearBuffers();
                            activeModel=loadIQM(path);
                            uploadBuffers();
                            animtime=.0f;
                        }
                    }
                    if(ImGui::MenuItem("Open Texture")){
                        const char* filter="*.png";
                        const char* path=tinyfd_openFileDialog(NULL,NULL,1,&filter,NULL,0);
                        if(path!=NULL){
                            activeModel.texture=loadTexture(path);
                            activeModel.textured=true;
                        }
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMainMenuBar();
            }



            //timeline
            ImGui::SetNextWindowPos(ImVec2(mainViewport->WorkPos.x+width/2.0f-300.0f,mainViewport->WorkPos.y+height-100.0f),true);
            
            if(ImGui::Begin("Timeline",NULL,ImGuiWindowFlags_AlwaysAutoResize)){
            
                const char* animationDropdownText=(activeModel.currentClip>=0 ? activeModel.clipNames[activeModel.currentClip].c_str():"[all]");
                if(activeModel.animatable&&ImGui::BeginCombo("##animselector",animationDropdownText)){
                    for(int i=0;i<activeModel.clipNames.size();i++){
                        bool isSelected=(i==activeModel.currentClip);
                        if(ImGui::Selectable(activeModel.clipNames[i].c_str(),isSelected))
                            activeModel.currentClip=i;
                        if(isSelected)
                            ImGui::SetItemDefaultFocus();
                    }
                    bool isSelected=-1;
                    if(ImGui::Selectable("[all]",isSelected))
                            activeModel.currentClip=-1;
                    if(isSelected)
                        ImGui::SetItemDefaultFocus();

                    ImGui::EndCombo();
                }
                ImGui::SameLine();
                ImGui::Text("%d vertices, %.2f FPS",activeModel.animationData.baseNormals.size(),io.Framerate);
                const char* pauseLabels[2]={"Pause","Unpause"};
                if(ImGui::Button(pauseLabels[paused]))
                    paused=!paused;
                ImGui::SameLine();
                if(activeModel.animatable)
                    if(ImGui::SliderFloat("##timeline",&animtime,0.0f,animationDuration))
                        seeking=true;
                ImGui::End();
            }
        }

        if(activeModel.animatable&&(!paused||seeking))
            activeModel.animate(animtime);


        //Rendering
        ImGui::Render();
        
        //adapt to window resize
        setAspectRatio((float)width/height);
        glViewport(0,0,width,height);        

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        activeModel.draw();

        //cleanup opengl state for ImGui
        glUseProgram(0);
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER,0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);

        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
