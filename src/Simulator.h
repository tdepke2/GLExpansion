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
#include "Configuration.h"
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
    static constexpr int ATTRIBUTE_LOCATION_V_POSITION = 0, ATTRIBUTE_LOCATION_V_NORMAL = 1, ATTRIBUTE_LOCATION_V_TEX_COORDS = 2, ATTRIBUTE_LOCATION_V_TANGENT = 3, ATTRIBUTE_LOCATION_V_BITANGENT = 4;
    
    static int start();
    static float randomFloat(float min = 0.0f, float max = 1.0f);    // Generates a random float between min (inclusive) and max (exclusive).
    static int randomInt(int min, int max);    // Generates a random integer between min and max inclusive.
    static GLenum glCheckError_(const char* file, int line);    // Error checking, https://learnopengl.com/In-Practice/Debugging
    static unsigned int loadTexture(const string& filename, bool gammaCorrection, bool flip = true);
    static unsigned int loadCubemap(const string& filename, bool gammaCorrection, bool flip = false);
    static unsigned int generateTexture(int r, int g, int b);
    
    private:
    enum class State {
        Uninitialized, Running, Paused, Exiting
    };
    
    static atomic<State> state;
    static mt19937 mainRNG;
    static Camera camera;
    static Configuration config;
    static glm::ivec2 windowSize;
    static glm::vec2 lastMousePos;
    static unordered_map<string, unsigned int> loadedTextures;
    static unique_ptr<Shader> geometryNormalMapShader, lightingPassShader, skyboxShader, lampShader, shadowMapShader, textRenderShader;
    static unique_ptr<Shader> postProcessShader, bloomShader, gaussianBlurShader, ssaoShader, ssaoBlurShader;
    static unique_ptr<Framebuffer> geometryFBO, renderFBO, shadowFBO;
    static unique_ptr<Framebuffer> bloom1FBO, bloom2FBO, ssaoFBO, ssaoBlurFBO;
    static unsigned int blackTexture, whiteTexture, blueTexture, cubeDiffuseMap, cubeSpecularMap, woodTexture, skyboxCubemap, brickDiffuseMap, brickNormalMap, ssaoNoiseTexture;
    static unsigned int viewProjectionMtxUBO;
    static Mesh lightCube, cube1, sphere1, windowQuad, skybox;
    static Model modelTest, planetModel, rockModel;
    static bool flashlightOn, sunlightOn, lampsOn, test;
    static float sunT, sunSpeed;
    
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    
    static GLFWwindow* setupOpenGL();
    static void setupTextures();
    static void setupShaders();
    static void setupBuffers();
    static void setupSimulation();
    static void nextTick(GLFWwindow* window);
    static void renderScene(const glm::mat4& viewMtx, const glm::mat4& projectionMtx, bool shadowRender, const glm::mat4& lightSpaceMtx);
    static void processInput(GLFWwindow* window, float deltaTime);
    
    friend class Configuration;
};

#endif
