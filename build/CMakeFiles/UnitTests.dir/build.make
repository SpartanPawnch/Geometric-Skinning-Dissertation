# CMAKE generated file: DO NOT EDIT!
# Generated by "MinGW Makefiles" Generator, CMake Version 3.22

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = "C:\Program Files\CMake\bin\cmake.exe"

# The command to remove a file.
RM = "C:\Program Files\CMake\bin\cmake.exe" -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = C:\Users\marti\Documents\DissProject

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = C:\Users\marti\Documents\DissProject\build

# Include any dependencies generated for this target.
include CMakeFiles/UnitTests.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/UnitTests.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/UnitTests.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/UnitTests.dir/flags.make

CMakeFiles/UnitTests.dir/src/unittests.cpp.obj: CMakeFiles/UnitTests.dir/flags.make
CMakeFiles/UnitTests.dir/src/unittests.cpp.obj: CMakeFiles/UnitTests.dir/includes_CXX.rsp
CMakeFiles/UnitTests.dir/src/unittests.cpp.obj: ../src/unittests.cpp
CMakeFiles/UnitTests.dir/src/unittests.cpp.obj: CMakeFiles/UnitTests.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\Users\marti\Documents\DissProject\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/UnitTests.dir/src/unittests.cpp.obj"
	C:\Users\marti\mingw64\bin\x86_64-w64-mingw32-g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/UnitTests.dir/src/unittests.cpp.obj -MF CMakeFiles\UnitTests.dir\src\unittests.cpp.obj.d -o CMakeFiles\UnitTests.dir\src\unittests.cpp.obj -c C:\Users\marti\Documents\DissProject\src\unittests.cpp

CMakeFiles/UnitTests.dir/src/unittests.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/UnitTests.dir/src/unittests.cpp.i"
	C:\Users\marti\mingw64\bin\x86_64-w64-mingw32-g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Users\marti\Documents\DissProject\src\unittests.cpp > CMakeFiles\UnitTests.dir\src\unittests.cpp.i

CMakeFiles/UnitTests.dir/src/unittests.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/UnitTests.dir/src/unittests.cpp.s"
	C:\Users\marti\mingw64\bin\x86_64-w64-mingw32-g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\Users\marti\Documents\DissProject\src\unittests.cpp -o CMakeFiles\UnitTests.dir\src\unittests.cpp.s

# Object files for target UnitTests
UnitTests_OBJECTS = \
"CMakeFiles/UnitTests.dir/src/unittests.cpp.obj"

# External object files for target UnitTests
UnitTests_EXTERNAL_OBJECTS =

UnitTests.exe: CMakeFiles/UnitTests.dir/src/unittests.cpp.obj
UnitTests.exe: CMakeFiles/UnitTests.dir/build.make
UnitTests.exe: CMakeFiles/UnitTests.dir/linklibs.rsp
UnitTests.exe: CMakeFiles/UnitTests.dir/objects1.rsp
UnitTests.exe: CMakeFiles/UnitTests.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=C:\Users\marti\Documents\DissProject\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable UnitTests.exe"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\UnitTests.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/UnitTests.dir/build: UnitTests.exe
.PHONY : CMakeFiles/UnitTests.dir/build

CMakeFiles/UnitTests.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles\UnitTests.dir\cmake_clean.cmake
.PHONY : CMakeFiles/UnitTests.dir/clean

CMakeFiles/UnitTests.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" C:\Users\marti\Documents\DissProject C:\Users\marti\Documents\DissProject C:\Users\marti\Documents\DissProject\build C:\Users\marti\Documents\DissProject\build C:\Users\marti\Documents\DissProject\build\CMakeFiles\UnitTests.dir\DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/UnitTests.dir/depend

