## Build Instructions
The project can be built using CMake (3.0 or newer) in combination with a C++ compiler and
build system. The compiler is required to contain the following dependencies: OpenGL,
GLEW, GLFW and GLM. With this in mind the project can be built using the standard
procedure for CMake:
1. Make a new folder called "build" inside the top-level project directory and navigate inside
it.
2. Run "cmake .."
3. Run "make" or its equivalent for the target build system

## Prebuilt Versions
Prebuilt Windows and Linux builds of the software are available in the /redist folder

## User Manual

### Camera Controls
The camera view can be manipulated by holding down the right mouse button and moving the
mouse. By holding down the Shift key alongside the right mouse button, the camera can be
panned vertically or horizontally relative to its current orientation. Finally, mouse scrolling can
be used to zoom in or out.
### Timeline Controls
The Timeline window is used to control animation playback. The active animation clip can be
selected using the dropdown menu. The pause/unpause button is used to stop or resume
playback and the slider next to it is used to manipulate the current frame being played back.
### Skinning Controls
The Skinning window is used to control the active skinning techniques being applied. The top
dropdown menu can be used to select between CPU and GPU skinning. The lower dropdown is
instead used to select between three sets of skinning weights. Finally, Delta Mush can be
enabled using a checkbox when CPU skinning is selected.
### File Loading
File loading functionality can be accessed by clicking the "File" button on the left side of the
program’s top menubar. The "assets" folder included with the project contains a set of IQM
models which can be loaded using the "Open Model" option. The folder also contains a single
texture - "complexanimations-texture.png", which can be used with the models
"complexanimationss-15k.iqm", "complexanimations-60k.iqm", "complexanimations-240k.iqm"
and "complexanimations-1m.iqm".
