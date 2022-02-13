cmake_minimum_required(VERSION 3.0)
project(SkinningDiss CXX C)

#setup Imgui
set(IMGUIDIR "external/imgui")
add_library(IMGUI STATIC)

target_sources(IMGUI
    PRIVATE
    ${IMGUIDIR}/imgui.cpp
    ${IMGUIDIR}/imgui_draw.cpp
    ${IMGUIDIR}/imgui_widgets.cpp
    ${IMGUIDIR}/imgui_tables.cpp

    PRIVATE
    ${IMGUIDIR}/backends/imgui_impl_opengl2.cpp
    ${IMGUIDIR}/backends/imgui_impl_glfw.cpp
)
target_include_directories(IMGUI
    PUBLIC ${IMGUIDIR}
    PUBLIC ${IMGUIDIR}/backends
)

set(NFDDIR "external/nativefiledialog/src")
add_library(NFD STATIC)

target_sources(NFD
    PRIVATE
    ${NFDDIR}/nfd_common.c
    ${NFDDIR}/nfd_win.cpp
)
target_include_directories(NFD
    PUBLIC ${NFDDIR}
    PUBLIC ${NFDDIR}/include
)

#setup executables
add_executable(${PROJECT_NAME} src/main.cpp src/animation.cpp src/modelloader.cpp)
add_executable(UnitTests src/unittests.cpp)

#compatibility
if(WIN32)
    message(STATUS "Using Windows library targets")
    target_link_libraries(${PROJECT_NAME} glfw3 opengl32 glew32)
    target_link_libraries(UnitTests glfw3 opengl32 glew32)
    target_link_libraries(IMGUI opengl32)

    if(CMAKE_BUILD_TYPE==RELEASE)
        #hide console
        set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-mwindows")
    endif()

else()
    message(STATUS "Using Linux library targets")
    target_link_libraries(${PROJECT_NAME} glfw GL GLEW)
    target_link_libraries(UnitTests glfw GL GLEW)
    target_link_libraries(IMGUI GL)
endif()
target_link_libraries(${PROJECT_NAME} IMGUI NFD)
target_compile_definitions(${PROJECT_NAME} PUBLIC ROOTDIR="${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_definitions(UnitTests PUBLIC ROOTDIR="${CMAKE_CURRENT_SOURCE_DIR}")

