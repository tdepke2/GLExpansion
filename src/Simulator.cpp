#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Shader.h"
#include "Simulator.h"
#include "VertexArrayObject.h"
#include <cassert>
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <utility>

atomic<Simulator::State> Simulator::state = {State::Uninitialized};
mt19937 Simulator::mainRNG;
Camera Simulator::camera(glm::vec3(0.0f, 0.0f, 3.0f));
glm::ivec2 Simulator::windowSize(800, 600);
glm::vec2 Simulator::lastMousePos(windowSize.x / 2.0f, windowSize.y / 2.0f);

int Simulator::start() {
    cout << "Initializing setup...\n";
    int exitCode = 0;
    GLFWwindow* window = nullptr;
    try {
        assert(state == State::Uninitialized);
        state = State::Running;
        mainRNG.seed(static_cast<unsigned long>(chrono::high_resolution_clock::now().time_since_epoch().count()));
        stbi_set_flip_vertically_on_load(true);
        
        window = setupOpenGL();
        setupShaders();
        setupTextures();
        setupSimulation();
        
        unsigned int diffuseMap = loadTexture("textures/container2.png");
        unsigned int specularMap = loadTexture("textures/container2_specular.png");
        
        Shader lightShader("shaders/phongShader.v.glsl", "shaders/lightShader.f.glsl");
        const int NUM_POINT_LIGHTS = 4;
        glm::vec3 pointLightPositions[NUM_POINT_LIGHTS] = {
            {0.7f, 0.2f, 2.0f},
            {2.3f, -3.3f, -4.0f},
            {-4.0f, 2.0f, -12.0f},
            {0.0f, 0.0f, -3.0f}
        };
        glm::vec3 pointLightColors[NUM_POINT_LIGHTS] = {
            {1.0f, 1.0f, 1.0f},
            {1.0f, 1.0f, 1.0f},
            {1.0f, 1.0f, 1.0f},
            {1.0f, 1.0f, 1.0f}
        };
        VertexArrayObject lightCube;
        lightCube.generateCube(0.2f);
        
        Shader cubeShader("shaders/phongShader.v.glsl", "shaders/phongShader.f.glsl");
        VertexArrayObject cube1;
        cube1.generateCube();
        
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        
        glm::vec3 cubePositions[] = {
            glm::vec3( 0.0f,  0.0f,  0.0f),
            glm::vec3( 2.0f,  5.0f, -15.0f),
            glm::vec3(-1.5f, -2.2f, -2.5f),
            glm::vec3(-3.8f, -2.0f, -12.3f),
            glm::vec3( 2.4f, -0.4f, -3.5f),
            glm::vec3(-1.7f,  3.0f, -7.5f),
            glm::vec3( 1.3f, -2.0f, -2.5f),
            glm::vec3( 1.5f,  2.0f, -2.5f),
            glm::vec3( 1.5f,  0.2f, -1.5f),
            glm::vec3(-1.3f,  1.0f, -1.5f)
        };
        
        double lastTime = glfwGetTime();
        float deltaTime = 0.0f;
        double lastFrameTime = lastTime;
        int frameCounter = 0;
        cout << "Setup complete.\n";
        while (state != State::Exiting) {    // Render loop.
            double currentTime = glfwGetTime();
            deltaTime = static_cast<float>(currentTime - lastTime);
            lastTime = currentTime;
            
            ++frameCounter;
            if (currentTime - lastFrameTime >= 1.0) {
                cout << 1000.0 / frameCounter << " ms/frame\n";
                frameCounter = 0;
                lastFrameTime += 1.0;
            }
            
            processInput(window, deltaTime);
            
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            glm::mat4 viewMtx = camera.getViewMatrix();
            glm::mat4 projectionMtx = glm::perspective(glm::radians(camera.fov), static_cast<float>(windowSize.x) / windowSize.y, NEAR_PLANE, FAR_PLANE);
            
            //lightColor.r = sin(glfwGetTime() * 2.0f);
            //lightColor.g = sin(glfwGetTime() * 0.7f);
            //lightColor.b = sin(glfwGetTime() * 1.3f);
            
            lightShader.use();
            lightShader.setMat4("viewMtx", viewMtx);
            lightShader.setMat4("projectionMtx", projectionMtx);
            
            for (int i = 0; i < NUM_POINT_LIGHTS; ++i) {
                lightShader.setVec3("lightColor", pointLightColors[i]);
                lightShader.setMat4("modelMtx", glm::translate(glm::mat4(1.0f), pointLightPositions[i]));
                lightCube.draw();
            }
            
            cubeShader.use();
            cubeShader.setInt("material.diffuse", 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, diffuseMap);
            cubeShader.setInt("material.specular", 1);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, specularMap);
            cubeShader.setFloat("material.shininess", 32.0f);
            
            cubeShader.setVec3("directionalLight.directionViewSpace", viewMtx * glm::vec4(-0.2f, -1.0f, -0.3f, 0.0f));
            glm::vec3 directionalLightColor(1.0f, 1.0f, 1.0f);
            cubeShader.setVec3("directionalLight.ambient", directionalLightColor * 0.05f);
            cubeShader.setVec3("directionalLight.diffuse", directionalLightColor * 0.4f);
            cubeShader.setVec3("directionalLight.specular", directionalLightColor * 0.5f);
            
            for (int i = 0; i < NUM_POINT_LIGHTS; ++i) {
                cubeShader.setVec3("pointLights[" + to_string(i) + "].positionViewSpace", viewMtx * glm::vec4(pointLightPositions[i], 1.0f));
                cubeShader.setVec3("pointLights[" + to_string(i) + "].ambient", pointLightColors[i] * 0.05f);
                cubeShader.setVec3("pointLights[" + to_string(i) + "].diffuse", pointLightColors[i] * 0.8f);
                cubeShader.setVec3("pointLights[" + to_string(i) + "].specular", pointLightColors[i]);
                cubeShader.setVec3("pointLights[" + to_string(i) + "].attenuationVals", glm::vec3(1.0f, 0.09f, 0.032f));
            }
            
            cubeShader.setVec3("spotLight.positionViewSpace", viewMtx * glm::vec4(camera.position, 1.0f));
            cubeShader.setVec3("spotLight.directionViewSpace", viewMtx * glm::vec4(camera.front, 0.0f));
            glm::vec3 spotLightColor(1.0f, 1.0f, 1.0f);
            cubeShader.setVec3("spotLight.ambient", spotLightColor * 0.0f);
            cubeShader.setVec3("spotLight.diffuse", spotLightColor);
            cubeShader.setVec3("spotLight.specular", spotLightColor);
            cubeShader.setVec3("spotLight.attenuationVals", glm::vec3(1.0f, 0.09f, 0.032f));
            cubeShader.setVec2("spotLight.cutOff", glm::vec2(glm::cos(glm::radians(12.5f)), glm::cos(glm::radians(17.5f))));
            
            cubeShader.setMat4("viewMtx", viewMtx);
            cubeShader.setMat4("projectionMtx", projectionMtx);
            
            for (unsigned int i = 0; i < 10; ++i) {
                glm::mat4 modelMtx = glm::translate(glm::mat4(1.0f), cubePositions[i]);
                float angle = 20.0f * i;
                modelMtx = glm::rotate(modelMtx, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
                cubeShader.setMat4("modelMtx", modelMtx);
                
                cube1.draw();
            }
            cubeShader.setMat4("modelMtx", glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, -0.5f, 3.0f)), glm::vec3(3.0f, 0.2f, 5.0f)));
            cube1.draw();
            
            glfwSwapBuffers(window);
            glfwPollEvents();
            glCheckError();
            
            if (glfwWindowShouldClose(window)) {
                state = State::Exiting;
            }
        }
    } catch (exception& ex) {
        state = State::Exiting;
        cout << "\n****************************************************\n";
        cout << "* A fatal error has occurred, terminating program. *\n";
        cout << "****************************************************\n";
        cout << "Error: " << ex.what() << "\n";
        cout << "(Press enter)\n";
        cin.get();
        exitCode = -1;
    }
    
    glfwDestroyWindow(window);
    glfwTerminate();
    return exitCode;
}

float Simulator::randomFloat(float min, float max) {
    uniform_real_distribution<float> minMaxRange(min, max);
    return minMaxRange(mainRNG);
}

int Simulator::randomInt(int min, int max) {
    uniform_int_distribution<int> minMaxRange(min, max);
    return minMaxRange(mainRNG);
}

GLenum Simulator::glCheckError_(const char* file, int line) {
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        string error;
        switch (errorCode) {
            case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
            case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
            case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
            case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
            default:                               error = "UNKNOWN ERROR (" + to_string(errorCode) + ")"; break;
        }
        cout << error << " | " << file << " (" << line << ")";
    }
    return errorCode;
}

void Simulator::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    windowSize.x = width;
    windowSize.y = height;
    glViewport(0, 0, width, height);
}

void Simulator::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_ESCAPE) {
            if (state == State::Running) {
                state = State::Paused;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                glfwSetCursorPos(window, windowSize.x / 2.0f, windowSize.y / 2.0f);
            } else if (state == State::Paused) {
                state = State::Running;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }
        }
    }
}

void Simulator::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    
}

void Simulator::cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    if (state == State::Running) {
        camera.processMouseMove(static_cast<float>(xpos) - lastMousePos.x, lastMousePos.y - static_cast<float>(ypos));
    }
    lastMousePos.x = static_cast<float>(xpos);
    lastMousePos.y = static_cast<float>(ypos);
}

void Simulator::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.processMouseScroll(static_cast<float>(xoffset), static_cast<float>(yoffset));
}

GLFWwindow* Simulator::setupOpenGL() {
    glfwInit();    // Initialize GLFW and set version.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    GLFWwindow* window = glfwCreateWindow(windowSize.x, windowSize.y, "LearnOpenGL", nullptr, nullptr);    // Create the window.
    if (window == nullptr) {
        throw runtime_error("Error: Failed to create GLFW window.");
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);    // Update our screen after at least 1 screen refresh.
    
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);    // Set function callbacks.
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetScrollCallback(window, scrollCallback);
    
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {    // Load all OpenGL function pointers with GLAD.
        throw runtime_error("Error: Failed to initialize GLAD.");
    }
    
    glEnable(GL_DEPTH_TEST);
    //glDepthFunc(GL_LESS);
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);
    //glFrontFace(GL_CCW);
    //glCullFace(GL_BACK);
    //glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    glfwSetCursorPos(window, windowSize.x / 2.0f, windowSize.y / 2.0f);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    return window;
}

void Simulator::setupShaders() {
    
}

void Simulator::setupTextures() {
    
}

void Simulator::setupSimulation() {
    
}

void Simulator::nextTick(GLFWwindow* window) {
    
}

void Simulator::renderScene(const glm::mat4& viewMtx, const glm::mat4& projectionMtx, float deltaTime) {
    
}

unsigned int Simulator::loadTexture(const string& filename) {
    unsigned int texHandle;
    glGenTextures(1, &texHandle);
    glBindTexture(GL_TEXTURE_2D, texHandle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    int width, height, numChannels;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &numChannels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, numChannels == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        cout << "Error: Unable to load texture \"" << filename << "\".\n";
    }
    stbi_image_free(data);
    
    return texHandle;
}

void Simulator::processInput(GLFWwindow* window, float deltaTime) {
    //if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        //glfwSetWindowShouldClose(window, true);
    //}
    
    glm::vec3 moveDirection(0.0f, 0.0f, 0.0f);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        moveDirection.z -= 1.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        moveDirection.z += 1.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        moveDirection.x -= 1.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        moveDirection.x += 1.0f;
    }
    if (moveDirection != glm::vec3(0.0f, 0.0f, 0.0f)) {
        camera.processKeyboard(glm::normalize(moveDirection), deltaTime);
    }
}
