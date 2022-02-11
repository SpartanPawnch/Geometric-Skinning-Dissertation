#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_opengl2.h>
#include <imgui_impl_glfw.h>


#include <iostream>

#define GLM_FORCE_PRECISION_LOWP_FLOAT
#define GLM_FORCE_INLINE

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

    //load models
    Model longboi = loadIQM(ROOTDIR "/assets/longboi.iqm");
    longboi.texture = loadTexture(ROOTDIR "/assets/longboi_texture.png");
    longboi.textured = true;
    uploadBuffers();
    longboi.currentClip=1;

    float animtime=.0f,lasttime=glfwGetTime();
    bool paused=false;    
    while (!glfwWindowShouldClose(window)) {
        bool seeking=false;

        //State Processing
        float frametime=glfwGetTime()-lasttime;
        lasttime+=frametime;
        animtime+=!paused*frametime*longboi.clips[longboi.currentClip].framerate;
        animtime=fmodf(animtime,longboi.clips[longboi.currentClip].length);


        //UI Input
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Timeline",nullptr,ImGuiWindowFlags_AlwaysAutoResize);

        if(ImGui::BeginCombo("##animselector",longboi.clipNames[longboi.currentClip].c_str())){
            for(int i=0;i<longboi.clipNames.size();i++){
                bool isSelected=(i==longboi.currentClip);
                if(ImGui::Selectable(longboi.clipNames[i].c_str(),isSelected)){
                    longboi.currentClip=i;
                }
                if(isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        ImGui::SameLine();
        ImGui::Text("%d vertices, %.2f FPS",longboi.animationData.baseNormals.size(),io.Framerate);
        const char* pauseLabels[2]={"Pause","Unpause"};

        if(ImGui::Button(pauseLabels[paused]))
            paused=!paused;
        ImGui::SameLine();
        if(ImGui::SliderFloat("##timeline",&animtime,0.0f,
            longboi.clips[longboi.currentClip].length)){
            seeking=true;
        }
        ImGui::End();


        if(!paused||seeking)
            longboi.animate(animtime);


        //Rendering
        ImGui::Render();
        
        //adapt to window resize
        int width,height;
        glfwGetFramebufferSize(window,&width,&height);
        setAspectRatio((float)width/height);
        glViewport(0,0,width,height);        

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        longboi.draw();

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
