# Compilo la aplicación KonohaApp con C++17 y OpenGL 4.1 en macOS.
CXX := clang++
CC := clang
.DEFAULT_GOAL := all

GLFW := $(shell brew --prefix glfw)
GLM := $(shell brew --prefix glm)
ASSIMP := $(shell brew --prefix assimp)

CPPFLAGS := -Iinclude -I. -Iglad/include -I$(GLFW)/include -I$(GLM)/include -I$(ASSIMP)/include
CXXFLAGS := -std=c++17 -O2 -MMD -MP
CFLAGS := -std=c11 -O2
LDFLAGS := -L$(GLFW)/lib -L$(ASSIMP)/lib
LDLIBS := -lglfw -lassimp -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo

SOURCES := src/main.cpp src/Camera.cpp src/Shader.cpp src/Model.cpp src/BezierCurve.cpp src/Scene.cpp
OBJECTS := $(patsubst src/%.cpp,build/%.o,$(SOURCES))
-include $(OBJECTS:.o=.d)

.PHONY: all run clean
all: KonohaApp

KonohaApp: $(OBJECTS) build/glad.o
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

build/%.o: src/%.cpp
	@mkdir -p build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

# GLAD está escrito en C y lo compilo con clang, no con clang++.
build/glad.o: glad/src/glad.c
	@mkdir -p build
	$(CC) -Iglad/include $(CFLAGS) -c $< -o $@

run: KonohaApp
	./KonohaApp

clean:
	rm -rf build KonohaApp
