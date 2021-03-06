#ifndef RENDER_APP_H_
#define RENDER_APP_H_

class Camera;
class Framebuffer;
class PerformanceMonitor;
class Scene;
class Shader;
class World;

#include <glad/glad.h>    // OpenGL includes.
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define glCheckError() RenderApp::glCheckError_(__FILE__, __LINE__)

#include "Configuration.h"
#include "Event.h"
#include "Mesh.h"
#include <atomic>
#include <memory>
#include <queue>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

class RenderApp {
    public:
    enum State {    // why not enum class like RenderState::Uninitialized? ####################################################################
        Uninitialized, Running, Paused, Exiting
    };
    
    static constexpr glm::ivec2 INITIAL_WINDOW_SIZE = glm::ivec2(800, 600);
    static constexpr float NEAR_PLANE = 0.1f, FAR_PLANE = 100.0f;
    static constexpr unsigned int NUM_CASCADED_SHADOWS = 3;
    static constexpr unsigned int ATTRIBUTE_LOCATION_V_POSITION = 0;
    static constexpr unsigned int ATTRIBUTE_LOCATION_V_NORMAL = 1;
    static constexpr unsigned int ATTRIBUTE_LOCATION_V_TEX_COORDS = 2;
    static constexpr unsigned int ATTRIBUTE_LOCATION_V_TANGENT = 3;
    static constexpr unsigned int ATTRIBUTE_LOCATION_V_BITANGENT = 4;
    static constexpr unsigned int ATTRIBUTE_LOCATION_V_BONE = 5;
    static constexpr unsigned int ATTRIBUTE_LOCATION_V_WEIGHT = 6;
    Configuration config_;
    
    static GLenum glCheckError_(const char* file, int line);    // Error checking, https://learnopengl.com/In-Practice/Debugging
    static unsigned int loadTexture(const string& filename, bool gammaCorrection, bool flip = true);    // consider changing to const char * for performance lookups. ##############################################
    static unsigned int loadTextureHDR(const string& filename, bool flip = true);
    static unsigned int loadCubemap(const string& filename, bool gammaCorrection, bool flip = false);
    static unsigned int generateTexture(float r, float g, float b, float a = 1.0f);
    RenderApp();
    ~RenderApp();
    State getState() const;
    void setState(State state);
    GLFWwindow* getWindowHandle() const;
    Scene* getScene() const;
    void init();
    Scene* createScene();
    void startRenderThread();
    void tempRender();
    void drawWorld();    // Applies each stage of the rendering pipeline to draw the scene.
    bool pollEvent(Event& e);    // Grab the next event from the event queue.
    void resizeBuffers(int width, int height);    // Resize the internal render buffers used for drawing to the window.
    void close();    // Clean up attached objects and destroy window.
    
    private:
    static bool instantiated_;
    static unordered_map<string, unsigned int> loadedTextures_;
    static queue<Event> eventQueue_;
    atomic<State> state_;
    mt19937 randNumGenerator_;
    GLFWwindow* window_;
    glm::ivec2 windowSize_;
    unique_ptr<Scene> scene_;
    unordered_map<const char*, PerformanceMonitor*> performanceMonitors_;
    unique_ptr<Shader> geometryShader_, geometryNormalMapShader_, geometrySkinningShader_, skyboxShader_, lampShader_, shadowMapShader_, shadowMapSkinningShader_, debugVectorsShader_, forwardRenderShader_, forwardPBRShader_;
    unique_ptr<Shader> nullLightShader_, directionalLightShader_, pointLightShader_, spotLightShader_, postProcessShader_, bloomShader_, gaussianBlurShader_, ssaoShader_, ssaoBlurShader_;
    unique_ptr<Shader> textShader_, shapeShader_;
    unique_ptr<Shader> equirectToCubeShader_, radianceConvolutionShader_, prefilterEnvShader_, integrateBRDFShader_;
    unique_ptr<Framebuffer> geometryFBO_, renderFBO_, cascadedShadowFBO_[NUM_CASCADED_SHADOWS];
    unique_ptr<Framebuffer> bloom1FBO_, bloom2FBO_, ssaoFBO_, ssaoBlurFBO_;
    unsigned int blackTexture_, whiteTexture_, blueTexture_, cubeDiffuseMap_, cubeSpecularMap_, woodTexture_, skyboxCubemap_, brickDiffuseMap_, brickNormalMap_, ssaoNoiseTexture_, monitorGridTexture_;
    unsigned int skyboxHDRTexture_, skyboxHDRCubemap_, irradianceCubemap_, prefilterEnvCubemap_, lookupBRDFTexture_, rustedIronAlbedo_, rustedIronNormal_, rustedIronMetallic_, rustedIronRoughness_;
    unsigned int viewProjectionMtxUBO_;
    Mesh windowQuad_, skybox_;
    float shadowZBounds_[NUM_CASCADED_SHADOWS + 1];
    double lastTime_, lastFrameTime_;
    int frameCounter_;
    glm::mat4 shadowProjections_[NUM_CASCADED_SHADOWS], viewToLightSpace_;
    
    static void windowCloseCallback(GLFWwindow* window);
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    void setupOpenGL();
    void setupTextures();
    void setupShaders();
    void setupBuffers();
    void setupRender();
    void beginFrame();    // Stages of the rendering pipeline.
    void drawShadowMaps(const Camera& camera, const World& world);
    void geometryPass(const Camera& camera, const World& world);
    void applySSAO();
    void lightingPass(const Camera& camera, const World& world);
    void drawLamps(const Camera& camera, const World& world);
    void drawSkybox();
    void applyBloom();
    void drawPostProcessing();
    void forwardLightingPass();
    void drawGUI();
    void endFrame();
    void renderScene(const Camera& camera, const World& world, const glm::mat4& viewMtx, const glm::mat4& projectionMtx, bool shadowRender);
    void renderScene2(const glm::mat4& viewMtx, const glm::mat4& projectionMtx);
    void processInput(float deltaTime);
    float randomFloat(float min = 0.0f, float max = 1.0f);    // Generates a random float between min (inclusive) and max (exclusive).
    int randomInt(int min, int max);    // Generates a random integer between min and max inclusive.
    
    friend class Configuration;
};

#endif
