# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.30

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

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/mohamed/Dev/C++/NikolEngine/examples

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/mohamed/Dev/C++/NikolEngine/examples/build

# Include any dependencies generated for this target.
include CMakeFiles/NikolExamples.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/NikolExamples.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/NikolExamples.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/NikolExamples.dir/flags.make

CMakeFiles/NikolExamples.dir/src/main.cpp.o: CMakeFiles/NikolExamples.dir/flags.make
CMakeFiles/NikolExamples.dir/src/main.cpp.o: /home/mohamed/Dev/C++/NikolEngine/examples/src/main.cpp
CMakeFiles/NikolExamples.dir/src/main.cpp.o: CMakeFiles/NikolExamples.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/mohamed/Dev/C++/NikolEngine/examples/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/NikolExamples.dir/src/main.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/NikolExamples.dir/src/main.cpp.o -MF CMakeFiles/NikolExamples.dir/src/main.cpp.o.d -o CMakeFiles/NikolExamples.dir/src/main.cpp.o -c /home/mohamed/Dev/C++/NikolEngine/examples/src/main.cpp

CMakeFiles/NikolExamples.dir/src/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/NikolExamples.dir/src/main.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/mohamed/Dev/C++/NikolEngine/examples/src/main.cpp > CMakeFiles/NikolExamples.dir/src/main.cpp.i

CMakeFiles/NikolExamples.dir/src/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/NikolExamples.dir/src/main.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/mohamed/Dev/C++/NikolEngine/examples/src/main.cpp -o CMakeFiles/NikolExamples.dir/src/main.cpp.s

# Object files for target NikolExamples
NikolExamples_OBJECTS = \
"CMakeFiles/NikolExamples.dir/src/main.cpp.o"

# External object files for target NikolExamples
NikolExamples_EXTERNAL_OBJECTS =

NikolExamples: CMakeFiles/NikolExamples.dir/src/main.cpp.o
NikolExamples: CMakeFiles/NikolExamples.dir/build.make
NikolExamples: /home/mohamed/Dev/C++/NikolEngine/build/libNikolCore.so
NikolExamples: CMakeFiles/NikolExamples.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/home/mohamed/Dev/C++/NikolEngine/examples/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable NikolExamples"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/NikolExamples.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/NikolExamples.dir/build: NikolExamples
.PHONY : CMakeFiles/NikolExamples.dir/build

CMakeFiles/NikolExamples.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/NikolExamples.dir/cmake_clean.cmake
.PHONY : CMakeFiles/NikolExamples.dir/clean

CMakeFiles/NikolExamples.dir/depend:
	cd /home/mohamed/Dev/C++/NikolEngine/examples/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/mohamed/Dev/C++/NikolEngine/examples /home/mohamed/Dev/C++/NikolEngine/examples /home/mohamed/Dev/C++/NikolEngine/examples/build /home/mohamed/Dev/C++/NikolEngine/examples/build /home/mohamed/Dev/C++/NikolEngine/examples/build/CMakeFiles/NikolExamples.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/NikolExamples.dir/depend

