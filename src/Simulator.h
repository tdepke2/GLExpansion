#ifndef _SIMULATOR_H
#define _SIMULATOR_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define glCheckError() glCheckError_(__FILE__, __LINE__)

#include "Camera.h"
#include <random>
#include <string>

using namespace std;

class Simulator {
    public:
    static int start();
    static int randomInteger(int min, int max);    // Generates a random integer between min and max inclusive.
    static int randomInteger(int n);    // Generates a random integer between 0 and n - 1.
    static GLenum glCheckError_(const char* file, int line);    // Error checking, https://learnopengl.com/In-Practice/Debugging
    
    private:
    static mt19937 mainRNG;
    static Camera camera;
    static glm::ivec2 windowSize;
    static glm::vec2 lastMousePos;
    
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    static unsigned int loadTexture(const string& filename);
    static void processInput(GLFWwindow* window, float deltaTime);
};

#endif
