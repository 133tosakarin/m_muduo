# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.21

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

# Produce verbose output by default.
VERBOSE = 1

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
CMAKE_COMMAND = /usr/local/bin/cmake

# The command to remove a file.
RM = /usr/local/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /root/m_muduo

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /root/m_muduo

# Include any dependencies generated for this target.
include CMakeFiles/test_blockingQueue.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/test_blockingQueue.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/test_blockingQueue.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/test_blockingQueue.dir/flags.make

CMakeFiles/test_blockingQueue.dir/test/test_blockingQueue.cc.o: CMakeFiles/test_blockingQueue.dir/flags.make
CMakeFiles/test_blockingQueue.dir/test/test_blockingQueue.cc.o: test/test_blockingQueue.cc
CMakeFiles/test_blockingQueue.dir/test/test_blockingQueue.cc.o: CMakeFiles/test_blockingQueue.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/root/m_muduo/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/test_blockingQueue.dir/test/test_blockingQueue.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/test_blockingQueue.dir/test/test_blockingQueue.cc.o -MF CMakeFiles/test_blockingQueue.dir/test/test_blockingQueue.cc.o.d -o CMakeFiles/test_blockingQueue.dir/test/test_blockingQueue.cc.o -c /root/m_muduo/test/test_blockingQueue.cc

CMakeFiles/test_blockingQueue.dir/test/test_blockingQueue.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/test_blockingQueue.dir/test/test_blockingQueue.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /root/m_muduo/test/test_blockingQueue.cc > CMakeFiles/test_blockingQueue.dir/test/test_blockingQueue.cc.i

CMakeFiles/test_blockingQueue.dir/test/test_blockingQueue.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/test_blockingQueue.dir/test/test_blockingQueue.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /root/m_muduo/test/test_blockingQueue.cc -o CMakeFiles/test_blockingQueue.dir/test/test_blockingQueue.cc.s

# Object files for target test_blockingQueue
test_blockingQueue_OBJECTS = \
"CMakeFiles/test_blockingQueue.dir/test/test_blockingQueue.cc.o"

# External object files for target test_blockingQueue
test_blockingQueue_EXTERNAL_OBJECTS =

bin/test_blockingQueue: CMakeFiles/test_blockingQueue.dir/test/test_blockingQueue.cc.o
bin/test_blockingQueue: CMakeFiles/test_blockingQueue.dir/build.make
bin/test_blockingQueue: lib/libdc.so
bin/test_blockingQueue: /usr/local/lib64/libyaml-cpp.a
bin/test_blockingQueue: CMakeFiles/test_blockingQueue.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/root/m_muduo/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable bin/test_blockingQueue"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/test_blockingQueue.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/test_blockingQueue.dir/build: bin/test_blockingQueue
.PHONY : CMakeFiles/test_blockingQueue.dir/build

CMakeFiles/test_blockingQueue.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/test_blockingQueue.dir/cmake_clean.cmake
.PHONY : CMakeFiles/test_blockingQueue.dir/clean

CMakeFiles/test_blockingQueue.dir/depend:
	cd /root/m_muduo && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /root/m_muduo /root/m_muduo /root/m_muduo /root/m_muduo /root/m_muduo/CMakeFiles/test_blockingQueue.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/test_blockingQueue.dir/depend
