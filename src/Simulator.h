#ifndef _SIMULATOR_H
#define _SIMULATOR_H

class Framebuffer;
class Shader;

#include <glad/glad.h>    // OpenGL includes.
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define glCheckError() Simulator::glCheckError_(__FILE__, __LINE__)

#include "Camera.h"
#include "Mesh.h"
#include "Model.h"
#include <atomic>
#include <memory>
#include <random>
#include <string>
#include <unordered_map>

using namespace std;

class Simulator {
    public:
    static constexpr float NEAR_PLANE = 0.1f, FAR_PLANE = 100.0f;
    static constexpr int ATTRIBUTE_LOCATION_V_POSITION = 0, ATTRIBUTE_LOCATION_V_NORMAL = 1, ATTRIBUTE_LOCATION_V_TEX_COORDS = 2;
    
    static int start();
    static float randomFloat(float min = 0.0f, float max = 1.0f);    // Generates a random float between min (inclusive) and max (exclusive).
    static int randomInt(int min, int max);    // Generates a random integer between min and max inclusive.
    static GLenum glCheckError_(const char* file, int line);    // Error checking, https://learnopengl.com/In-Practice/Debugging
    static unsigned int loadTexture(const string& filename, bool gammaCorrection);
    static unsigned int loadCubemap(const string& filename, bool gammaCorrection);
    static unsigned int generateTexture(int r, int g, int b);
    
    private:
    enum class State {
        Uninitialized, Running, Paused, Exiting
    };
    
    static atomic<State> state;
    static mt19937 mainRNG;
    static Camera camera;
    static glm::ivec2 windowSize;
    static glm::vec2 lastMousePos;
    static unordered_map<string, unsigned int> loadedTextures;
    static unique_ptr<Shader> skyboxShader, lightShader, phongShader, shadowMapShader, framebufferShader, testShader;
    static unique_ptr<Framebuffer> renderFramebuffer, shadowFramebuffer;
    static unsigned int blackTexture, whiteTexture, cubeDiffuseMap, cubeSpecularMap, woodTexture, skyboxCubemap;
    static unsigned int uniformBufferVPMtx;
    static Mesh lightCube, cube1, windowQuad, skybox;
    static Model modelTest, planetModel, rockModel;
    static bool flashlightOn, sunlightOn, lampsOn;
    static float sunT, sunSpeed;
    
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    
    static GLFWwindow* setupOpenGL();
    static void setupTextures();
    static void setupShaders();
    static void setupSimulation();
    static void nextTick(GLFWwindow* window);
    static void renderScene(const glm::mat4& viewMtx, const glm::mat4& projectionMtx, bool shadowRender, const glm::mat4& lightSpaceMtx);
    
    static void processInput(GLFWwindow* window, float deltaTime);
};

#endif
