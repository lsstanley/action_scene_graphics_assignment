# Compilers and flags
CXX = g++
CC = gcc
CXXFLAGS = -std=c++11 -I./include -I/opt/homebrew/opt/glfw/include -I../basic_structure/common -I../basic_structure/include -I../basic_structure/example_objects
CFLAGS = -I./include -I/opt/homebrew/opt/glfw/include -I../basic_structure/common -I../basic_structure/include -I../basic_structure/example_objects
LDFLAGS = -L/opt/homebrew/opt/glfw/lib -lglfw -framework OpenGL

# Bash script to find the first .cpp file in the directory
MAIN_CPP := $(shell find . -maxdepth 1 -name '*.cpp' | head -n 1)

# Additional source files (statically set)
ADDITIONAL_SRC = ../basic_structure/common/wrapper_glfw.cpp \
	../basic_structure/example_objects/sphere.cpp \
	../basic_structure/example_objects/cube.cpp \
	../basic_structure/example_objects/cylinder.cpp

# Combine the dynamically found file and the additional sources
CXXSRC = $(MAIN_CPP) $(ADDITIONAL_SRC)

# Get the output filename by stripping the .cpp extension from the main file
OUTPUT := $(basename $(notdir $(MAIN_CPP)))

# Source files
CSRC = ../basic_structure/common/glad.c
CXXOBJ = $(CXXSRC:.cpp=.o)
COBJ = $(CSRC:.c=.o)

# Default target
all: $(OUTPUT)

# Link the final executable
$(OUTPUT): $(CXXOBJ) $(COBJ)
	$(CXX) $(CXXOBJ) $(COBJ) $(LDFLAGS) -o $(OUTPUT)

# Compile C++ files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile C files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up the build
clean:
	rm -f $(CXXOBJ) $(COBJ) $(OUTPUT)

bear:
	@bear -- make clean all

.PHONY: all clean
