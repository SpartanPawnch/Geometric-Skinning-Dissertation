cmake_minimum_required(VERSION 3.0)
project(SkinningDiss CXX C)

#setup Imgui
set(IMGUIDIR "external/imgui")
# add_library(IMGUI STATIC)
include_directories(${IMGUIDIR} ${IMGUIDIR}/backends)
aux_source_directory("${IMGUIDIR}/" IMGUICORE)
set(IMGUIBACKENDS "${IMGUIDIR}/backends/imgui_impl_opengl2.cpp" "${IMGUIDIR}/backends/imgui_impl_glfw.cpp")



#setup executables
add_executable(${PROJECT_NAME} src/main.cpp src/animation.cpp src/modelloader.cpp external/tinyfiledialogs/tinyfiledialogs.c ${IMGUICORE} ${IMGUIBACKENDS})
add_executable(UnitTests src/unittests.cpp src/animation.cpp)

#compatibility
if(WIN32)
    message(STATUS "Using Windows library targets")
    target_link_libraries(${PROJECT_NAME} glfw3 opengl32 glew32)
    target_link_libraries(UnitTests glfw3 opengl32 glew32)

    if(CMAKE_BUILD_TYPE==RELEASE)
        #hide console
        set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-mwindows")
    endif()

else()
    message(STATUS "Using Linux library targets")

    #setup internal glfw
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    add_subdirectory("external/glfw-3.3.6")

    target_link_libraries(${PROJECT_NAME} glfw GL GLEW)
    target_link_libraries(UnitTests glfw GL GLEW)
endif()
# target_link_libraries(${PROJECT_NAME} IMGUI)
target_compile_definitions(${PROJECT_NAME} PUBLIC ROOTDIR="${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_definitions(UnitTests PUBLIC ROOTDIR="${CMAKE_CURRENT_SOURCE_DIR}")

