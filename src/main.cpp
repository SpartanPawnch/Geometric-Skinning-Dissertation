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
    GLFWwindow* window;
    glfwInit();

#if GLFW_VERSION_MAJOR>=3 && GLFW_VERSION_MINOR>=1
    //Supported by glfw 3.1 and onwards
    glfwWindowHint(GLFW_MAXIMIZED, 1);
#endif

    glfwWindowHint(GLFW_SAMPLES, 4);

    window = glfwCreateWindow(1600, 900, "Skinning Dissertation", NULL, NULL);
    //glfwSwapInterval(0);


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
    ImGuiIO& io = ImGui::GetIO();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL2_Init();

    const ImGuiViewport* mainViewport = ImGui::GetMainViewport();

    //load models
    Model activeModel = loadIQM(ROOTDIR "/assets/complexanimations-15k.iqm");
    activeModel.texture = loadTexture(ROOTDIR "/assets/complexanims_texture.png");
    activeModel.textured = true;
    activeModel.skinningType = SkinningTypeGPU;
    uploadBuffers();
    activeModel.currentClip = 0;

    //animation control variables
    float animtime = .0f, lasttime = glfwGetTime();
    bool paused = false;

    //camera control variables
    glm::vec2 lastMousePos;
    glm::vec2 currentMousePos;
    bool mouseWasDown = false;
    bool applyArcball = false;
    bool applyPan = false;

    //metrics
    float averageFPS = io.Framerate, minFPS = 144.0f, maxFPS = io.Framerate;
    long totalFrames = 1;

    while (!glfwWindowShouldClose(window)) {
        bool seeking = false;

        //State Processing
        float frametime = glfwGetTime() - lasttime;
        lasttime += frametime;

        float animationDuration = 1.0f, framerate = 60.0f;
        if (activeModel.currentClip >= 0) {
            framerate = activeModel.clips[activeModel.currentClip].framerate;
            animationDuration = activeModel.clips[activeModel.currentClip].length;
        }
        else if (activeModel.animatable) {
            animationDuration = ((float)activeModel.animationData.poses.size()) / activeModel.animationData.posesPerFrame;
        }
        animtime += !paused * frametime * framerate;
        animtime = fmodf(animtime, animationDuration);
        //get window parameters
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        sceneCamera.screenWidth = width;
        sceneCamera.screenHeight = height;


        //UI Input
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        {
            //top menubar
            if (ImGui::BeginMainMenuBar()) {
                if (ImGui::BeginMenu("File")) {
                    if (ImGui::MenuItem("Open Model")) {
                        const char* filter = "*.iqm";
                        const char* path = tinyfd_openFileDialog(NULL, NULL, 1, &filter, NULL, 0);
                        if (path != NULL) {
                            activeModel.clear();
                            clearBuffers();
                            activeModel = loadIQM(path);
                            uploadBuffers();
                            animtime = .0f;

                            //reset camera
                            sceneCamera = Camera(glm::vec3(.0f, 2.0f, 4.0f),
                                glm::vec3(.0f, 1.0f, .0f), glm::vec3(.0f, 1.0f, .0f));

                            //reset metrics
                            averageFPS = io.Framerate;
                            minFPS = 144.0f;
                            maxFPS = io.Framerate;
                            totalFrames = 1;
                        }
                    }
                    if (ImGui::MenuItem("Open Texture")) {
                        const char* filter = "*.png";
                        const char* path = tinyfd_openFileDialog(NULL, NULL, 1, &filter, NULL, 0);
                        if (path != NULL) {
                            activeModel.texture = loadTexture(path);
                            activeModel.textured = true;
                        }

                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMainMenuBar();
            }



            //timeline
            ImGui::SetNextWindowPos(ImVec2(mainViewport->WorkPos.x + width / 2.0f - 300.0f, mainViewport->WorkPos.y + height - 100.0f), ImGuiCond_Once);
            ImGui::SetNextWindowSize(ImVec2(500.0f, 100.0f), ImGuiCond_Once);
            if (ImGui::Begin("Timeline", NULL)) {

                const char* animationDropdownText = (activeModel.currentClip >= 0 ? activeModel.clipNames[activeModel.currentClip].c_str() : "[all]");
                if (activeModel.animatable && ImGui::BeginCombo("##animselector", animationDropdownText)) {
                    for (int i = 0;i < activeModel.clipNames.size();i++) {
                        bool isSelected = (i == activeModel.currentClip);
                        if (ImGui::Selectable(activeModel.clipNames[i].c_str(), isSelected))
                            activeModel.currentClip = i;
                        if (isSelected)
                            ImGui::SetItemDefaultFocus();
                    }
                    bool isSelected = false;
                    if (ImGui::Selectable("[all]", isSelected))
                        activeModel.currentClip = -1;
                    if (isSelected)
                        ImGui::SetItemDefaultFocus();

                    ImGui::EndCombo();
                }

                const char* pauseLabels[2] = { "Pause","Unpause" };
                if (ImGui::Button(pauseLabels[paused]))
                    paused = !paused;
                ImGui::SameLine();
                ImGui::SetNextItemWidth(ImGui::GetWindowWidth() - 70.0f);
                if (activeModel.animatable)
                    if (ImGui::SliderFloat("##timeline", &animtime, 0.0f, animationDuration))
                        seeking = true;
                ImGui::End();
            }

            //metrics window
            if (ImGui::Begin("Metrics", NULL)) {
                ImGui::Text("%d vertices\n%.2f FPS, %.4f ms\n%.2f Avg, %.2f Min, %.2f Max",
                    activeModel.animationData.baseNormals.size(), io.Framerate, io.DeltaTime, averageFPS, minFPS, maxFPS);
                ImGui::End();
            }
        }

        if (activeModel.animatable && (!paused || seeking))
            activeModel.animate(animtime);


        //Mouse Input
        {

            if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
                //record last position
                if (mouseWasDown) {
                    lastMousePos = currentMousePos;
                    if (ImGui::IsKeyDown(GLFW_KEY_LEFT_SHIFT) || ImGui::IsKeyDown(GLFW_KEY_RIGHT_SHIFT)) {
                        applyPan = true;
                        applyArcball = false;
                    }
                    else {
                        applyArcball = true;
                        applyPan = false;
                    }
                }
                ImVec2 pos = ImGui::GetMousePos();
                currentMousePos = glm::vec2(pos.x, pos.y);
                mouseWasDown = true;
            }
            else {
                applyArcball = false;
                applyPan = false;
                mouseWasDown = false;
            }



            //manipulate camera
            if (applyArcball) {
                sceneCamera.rotateArcball(lastMousePos, currentMousePos);
            }
            if (applyPan) {
                glm::vec2 mouseDelta = currentMousePos - lastMousePos;
                mouseDelta.x *= -1.0f;
                sceneCamera.pan(mouseDelta);
            }

            const float scrollScale = .05f;
            sceneCamera.zoom(scrollScale * io.MouseWheel);
        }


        //Rendering
        ImGui::Render();

        //adapt to window resize
        glViewport(0, 0, width, height);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        activeModel.draw();

        //cleanup opengl state for ImGui
        glUseProgram(0);
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();

        //update metrics
        totalFrames++;
        averageFPS = (averageFPS * (totalFrames - 1) + io.Framerate) / totalFrames;
        minFPS = io.Framerate < minFPS ? io.Framerate : minFPS;
        maxFPS = io.Framerate > maxFPS ? io.Framerate : maxFPS;
    }

    activeModel.clear();

    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
