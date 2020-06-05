#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Framebuffer.h"
#include "Shader.h"
#include "Simulator.h"
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
unordered_map<string, unsigned int> Simulator::loadedTextures;
unique_ptr<Shader> Simulator::skyboxShader, Simulator::lightShader, Simulator::phongShader, Simulator::framebufferShader, Simulator::testShader;
unique_ptr<Framebuffer> Simulator::renderFramebuffer;
unsigned int Simulator::blackTexture, Simulator::whiteTexture, Simulator::cubeDiffuseMap, Simulator::cubeSpecularMap, Simulator::woodTexture, Simulator::skyboxCubemap;
unsigned int Simulator::uniformBufferVPMtx;
Mesh Simulator::lightCube, Simulator::cube1, Simulator::windowQuad, Simulator::skybox;
Model Simulator::modelTest, Simulator::planetModel, Simulator::rockModel;
bool Simulator::flashlightOn = false, Simulator::sunlightOn = true, Simulator::lampsOn = false;

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
        setupTextures();
        setupShaders();
        setupSimulation();
        
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        
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
                string windowTitle = to_string(frameCounter) + " FPS (" + to_string(1000.0f / frameCounter) + " ms/frame)";
                glfwSetWindowTitle(window, windowTitle.c_str());
                frameCounter = 0;
                lastFrameTime += 1.0;
                if (currentTime - lastFrameTime >= 1.0) {
                    lastFrameTime = currentTime;
                }
            }
            
            processInput(window, deltaTime);
            
            renderFramebuffer->bind();    // Prepare framebuffer for drawing.
            glViewport(0, 0, renderFramebuffer->getBufferSize().x, renderFramebuffer->getBufferSize().y);
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glm::mat4 viewMtx = camera.getViewMatrix();
            glm::mat4 projectionMtx = glm::perspective(glm::radians(camera.fov), static_cast<float>(windowSize.x) / windowSize.y, NEAR_PLANE, FAR_PLANE);
            
            renderScene(viewMtx, projectionMtx, deltaTime);
            
            skyboxShader->use();    // Draw the skybox.
            glDepthFunc(GL_LEQUAL);
            glDisable(GL_CULL_FACE);
            skyboxShader->setInt("skybox", 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxCubemap);
            skybox.draw();
            glDepthFunc(GL_LESS);
            glEnable(GL_CULL_FACE);
            
            glBindFramebuffer(GL_FRAMEBUFFER, 0);    // Draw framebuffer.
            glViewport(0, 0, windowSize.x, windowSize.y);
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            glDisable(GL_DEPTH_TEST);
            framebufferShader->use();
            framebufferShader->setInt("tex", 0);
            renderFramebuffer->bindTexColorBuffer();
            windowQuad.draw();
            glEnable(GL_DEPTH_TEST);
            
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
    
    glDeleteBuffers(1, &uniformBufferVPMtx);    // Clean up allocated resources.
    skyboxShader.reset();
    lightShader.reset();
    phongShader.reset();
    framebufferShader.reset();
    testShader.reset();
    renderFramebuffer.reset();
    
    glCheckError();
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
        cout << error << " | " << file << " (" << line << ")\n";
    }
    return errorCode;
}

unsigned int Simulator::loadTexture(const string& filename) {
    auto findResult = loadedTextures.find(filename);
    if (findResult != loadedTextures.end()) {
        return findResult->second;
    }
    
    cout << "Loading texture \"" << filename << "\".\n";
    unsigned int texHandle;
    glGenTextures(1, &texHandle);
    glBindTexture(GL_TEXTURE_2D, texHandle);
    int width, height, numChannels = 1;
    unsigned char* imageData = stbi_load(filename.c_str(), &width, &height, &numChannels, 0);
    GLenum format;
    if (numChannels == 1) {
        format = GL_RED;
    } else if (numChannels == 3) {
        format = GL_RGB;
    } else if (numChannels == 4) {
        format = GL_RGBA;
    } else {
        cout << "Error: Unsupported channel number.\n";
        stbi_image_free(imageData);
        imageData = nullptr;
    }
    
    if (imageData) {
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, imageData);
        
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    } else {
        cout << "Error: Unable to load texture.\n";
    }
    stbi_image_free(imageData);
    
    loadedTextures[filename] = texHandle;
    return texHandle;
}

unsigned int Simulator::loadCubemap(const string& filename) {
    auto findResult = loadedTextures.find(filename);
    if (findResult != loadedTextures.end()) {
        return findResult->second;
    }
    
    stbi_set_flip_vertically_on_load(false);    // Need to flip the textures to match the specifications of a cubemap.
    string prefix = filename.substr(0, filename.find('.'));
    string postfix = filename.substr(filename.find('.'));
    cout << "Loading cubemap.\n";
    unsigned int texHandle;
    glGenTextures(1, &texHandle);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texHandle);
    
    int width, height, numChannels = 1;
    for (unsigned int i = 0; i < 6; ++i) {
        string faceFilename = prefix + (i % 2 == 0 ? "pos" : "neg");
        if (i < 2) {
            faceFilename += "x" + postfix;
        } else if (i < 4) {
            faceFilename += "y" + postfix;
        } else {
            faceFilename += "z" + postfix;
        }
        cout << "  Face " << i << ": \"" << faceFilename << "\".\n";
        
        unsigned char* imageData = stbi_load(faceFilename.c_str(), &width, &height, &numChannels, 0);
        GLenum format;
        if (numChannels == 1) {
            format = GL_RED;
        } else if (numChannels == 3) {
            format = GL_RGB;
        } else if (numChannels == 4) {
            format = GL_RGBA;
        } else {
            cout << "Error: Unsupported channel number.\n";
            stbi_image_free(imageData);
            imageData = nullptr;
        }
        if (imageData) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, imageData);
        } else {
            cout << "Error: Unable to load texture.\n";
        }
        stbi_image_free(imageData);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_set_flip_vertically_on_load(true);
    
    loadedTextures[filename] = texHandle;
    return texHandle;
}

unsigned int Simulator::generateTexture(int r, int g, int b) {
    string textureName = "color " + to_string(r) + " " + to_string(g) + " " + to_string(b);
    auto findResult = loadedTextures.find(textureName);
    if (findResult != loadedTextures.end()) {
        return findResult->second;
    }
    
    cout << "Generating texture \"" << textureName << "\".\n";
    unsigned int texHandle;
    glGenTextures(1, &texHandle);
    glBindTexture(GL_TEXTURE_2D, texHandle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    GLubyte* imageData = new GLubyte[8 * 8 * 3];
    for (int i = 0; i < 8 * 8 * 3; i += 3) {
        imageData[i] = static_cast<GLubyte>(r);
        imageData[i + 1] = static_cast<GLubyte>(g);
        imageData[i + 2] = static_cast<GLubyte>(b);
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 8, 8, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    loadedTextures[textureName] = texHandle;
    return texHandle;
}

void Simulator::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    windowSize.x = width;
    windowSize.y = height;
    renderFramebuffer->setBufferSize(windowSize);
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
        } else if (key == GLFW_KEY_UP) {
            camera.moveSpeed *= 2.0f;
        } else if (key == GLFW_KEY_DOWN) {
            if (camera.moveSpeed > 0.1f) {
                camera.moveSpeed /= 2.0f;
            }
        } else if (key == GLFW_KEY_F) {
            flashlightOn = !flashlightOn;
        } else if (key == GLFW_KEY_G) {
            sunlightOn = !sunlightOn;
        } else if (key == GLFW_KEY_H) {
            lampsOn = !lampsOn;
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
    glDepthFunc(GL_LESS);
    glDisable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    //glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    glfwSetCursorPos(window, windowSize.x / 2.0f, windowSize.y / 2.0f);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    return window;
}

void Simulator::setupTextures() {
    blackTexture = generateTexture(0, 0, 0);
    whiteTexture = generateTexture(255, 255, 255);
    cubeDiffuseMap = loadTexture("textures/container2.png");
    cubeSpecularMap = loadTexture("textures/container2_specular.png");
    woodTexture = loadTexture("textures/wood.png");
}

void Simulator::setupShaders() {
    glGenBuffers(1, &uniformBufferVPMtx);    // Create a uniform buffer for ViewProjectionMtx.
    glBindBuffer(GL_UNIFORM_BUFFER, uniformBufferVPMtx);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), nullptr, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, uniformBufferVPMtx, 0, 2 * sizeof(glm::mat4));    // Link to binding point 0.
    
    skyboxShader = make_unique<Shader>("shaders/skyboxShader.v.glsl", "shaders/skyboxShader.f.glsl");
    skyboxShader->setUniformBlockBinding("ViewProjectionMtx", 0);
    
    lightShader = make_unique<Shader>("shaders/phongShader.v.glsl", "shaders/lightShader.f.glsl");
    lightShader->setUniformBlockBinding("ViewProjectionMtx", 0);
    
    phongShader = make_unique<Shader>("shaders/phongShader.v.glsl", "shaders/phongShader.f.glsl");
    phongShader->setUniformBlockBinding("ViewProjectionMtx", 0);
    
    framebufferShader = make_unique<Shader>("shaders/framebufferShader.v.glsl", "shaders/framebufferShader.f.glsl");
    
    testShader = make_unique<Shader>("shaders/phongShader.v.glsl", "shaders/blendShader.f.glsl");
    testShader->setUniformBlockBinding("ViewProjectionMtx", 0);
}

void Simulator::setupSimulation() {
    renderFramebuffer = make_unique<Framebuffer>(windowSize);
    
    vector<Mesh::Vertex> windowQuadVertices = {
        { 1.0f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f, 1.0f, 1.0f},
        {-1.0f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f, 0.0f, 1.0f},
        {-1.0f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f, 0.0f, 0.0f},
        { 1.0f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f, 1.0f, 0.0f}
    };
    vector<unsigned int> windowQuadIndices = {
        0, 1, 2, 2, 3, 0
    };
    windowQuad.generateMesh(move(windowQuadVertices), move(windowQuadIndices));
    
    skyboxCubemap = loadCubemap("textures/skybox/.jpg");
    skybox.generateCube(2.0f);
    
    lightCube.generateCube(0.2f);
    cube1.generateCube();
    //stbi_set_flip_vertically_on_load(false);
    modelTest.loadFile("models/backpack/backpack.obj");
    //stbi_set_flip_vertically_on_load(true);
    
    // Instancing example.
    /*planetModel.loadFile("models/planet/planet.obj");
    rockModel.loadFile("models/rock/rock.obj");
    
    constexpr int NUM_ROCKS = 1000;
    glm::mat4* rockTransforms = new glm::mat4[NUM_ROCKS];
    constexpr float RADIUS = 50.0f, OFFSET = 2.5f;
    for (int i = 0; i < NUM_ROCKS; ++i) {
        glm::mat4 modelMtx(1.0f);
        float angle = static_cast<float>(i) / NUM_ROCKS * 360.0f;
        float x = sin(angle) * RADIUS + (randomInt(0, static_cast<int>(2 * OFFSET * 100 - 1)) / 100.0f - OFFSET);
        float y = (randomInt(0, static_cast<int>(2 * OFFSET * 100 - 1)) / 100.0f - OFFSET) * 0.4f;
        float z = cos(angle) * RADIUS + randomInt(0, static_cast<int>(2 * OFFSET * 100 - 1)) / 100.0f - OFFSET;
        modelMtx = glm::translate(modelMtx, glm::vec3(x, y, z));
        modelMtx = glm::scale(modelMtx, glm::vec3(randomInt(0, 19) / 100.0f + 0.05f));
        modelMtx = glm::rotate(modelMtx, randomFloat(0.0f, 360.0f), glm::vec3(0.4f, 0.6f, 0.8f));
        
        rockTransforms[i] = modelMtx;
    }
    
    unsigned int instanceBuffer;
    glGenBuffers(1, &instanceBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, instanceBuffer);
    glBufferData(GL_ARRAY_BUFFER, NUM_ROCKS * sizeof(glm::mat4), &rockTransforms[0], GL_STATIC_DRAW);
    rockModel.applyInstanceBuffer(3);
    delete[] rockTransforms;*/
}

void Simulator::nextTick(GLFWwindow* window) {
    
}

void Simulator::renderScene(const glm::mat4& viewMtx, const glm::mat4& projectionMtx, float deltaTime) {
    glBindBuffer(GL_UNIFORM_BUFFER, uniformBufferVPMtx);    // Update uniform buffer.
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(viewMtx));
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(projectionMtx));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    
    constexpr int NUM_LIGHTS = 8;
    glm::vec3 pointLightPositions[4] = {
        {0.7f, 0.2f, 2.0f},
        {2.3f, -3.3f, -4.0f},
        {-4.0f, 2.0f, -12.0f},
        {0.0f, 0.0f, -3.0f}
    };
    glm::vec3 pointLightColors[4] = {
        {1.0f, 1.0f, 1.0f},
        {1.0f, 1.0f, 1.0f},
        {1.0f, 1.0f, 1.0f},
        {1.0f, 1.0f, 1.0f}
    };
    unsigned int lightStates[NUM_LIGHTS] = {0, 0, 0, 0, 0, 0, 0, 0};
    lightStates[0] = (sunlightOn ? 1u : 0u);
    lightStates[1] = (flashlightOn ? 1u : 0u);
    for (int i = 0; i < 4; ++i) {
        lightStates[i + 2] = (lampsOn ? 1u : 0u);
    }
    
    lightShader->use();
    for (int i = 0; i < 4; ++i) {
        if (lightStates[i + 2] == 1) {
            lightShader->setVec3("lightColor", pointLightColors[i]);
            lightShader->setMat4("modelMtx", glm::translate(glm::mat4(1.0f), pointLightPositions[i]));
            lightCube.draw();
        }
    }
    
    phongShader->use();
    phongShader->setInt("material.texDiffuse0", 0);
    phongShader->setInt("material.texSpecular0", 1);
    phongShader->setFloat("material.shininess", 32.0f);
    
    phongShader->setUnsignedIntArray("lightStates", NUM_LIGHTS, lightStates);
    
    phongShader->setUnsignedInt("lights[0].type", 0);
    phongShader->setVec3("lights[0].directionViewSpace", viewMtx * glm::vec4(-0.2f, -1.0f, -0.3f, 0.0f));
    glm::vec3 directionalLightColor(1.0f, 1.0f, 1.0f);
    phongShader->setVec3("lights[0].ambient", directionalLightColor * 0.05f);
    phongShader->setVec3("lights[0].diffuse", directionalLightColor * 0.4f);
    phongShader->setVec3("lights[0].specular", directionalLightColor * 0.5f);
    
    phongShader->setUnsignedInt("lights[1].type", 2);
    phongShader->setVec3("lights[1].positionViewSpace", viewMtx * glm::vec4(camera.position, 1.0f));
    phongShader->setVec3("lights[1].directionViewSpace", viewMtx * glm::vec4(camera.front, 0.0f));
    glm::vec3 spotLightColor(1.0f, 1.0f, 1.0f);
    phongShader->setVec3("lights[1].ambient", spotLightColor * 0.0f);
    phongShader->setVec3("lights[1].diffuse", spotLightColor);
    phongShader->setVec3("lights[1].specular", spotLightColor);
    phongShader->setVec3("lights[1].attenuationVals", glm::vec3(1.0f, 0.09f, 0.032f));
    phongShader->setVec2("lights[1].cutOff", glm::vec2(glm::cos(glm::radians(12.5f)), glm::cos(glm::radians(17.5f))));
    
    for (int i = 0; i < 4; ++i) {
        phongShader->setUnsignedInt("lights[" + to_string(i + 2) + "].type", 1);
        phongShader->setVec3("lights[" + to_string(i + 2) + "].positionViewSpace", viewMtx * glm::vec4(pointLightPositions[i], 1.0f));
        phongShader->setVec3("lights[" + to_string(i + 2) + "].ambient", pointLightColors[i] * 0.05f);
        phongShader->setVec3("lights[" + to_string(i + 2) + "].diffuse", pointLightColors[i] * 0.8f);
        phongShader->setVec3("lights[" + to_string(i + 2) + "].specular", pointLightColors[i]);
        phongShader->setVec3("lights[" + to_string(i + 2) + "].attenuationVals", glm::vec3(1.0f, 0.09f, 0.032f));
    }
    
    phongShader->setMat4("modelMtx", glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 2.0f)), glm::vec3(0.9f, 0.9f, 0.9f)));
    modelTest.draw(*phongShader);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, cubeDiffuseMap);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, cubeSpecularMap);
    vector<glm::vec3> cubePositions = {
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
    for (unsigned int i = 0; i < cubePositions.size(); ++i) {
        glm::mat4 modelMtx = glm::translate(glm::mat4(1.0f), cubePositions[i]);
        float angle = 20.0f * i;
        modelMtx = glm::rotate(modelMtx, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
        phongShader->setMat4("modelMtx", modelMtx);
        cube1.draw();
    }
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, woodTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, whiteTexture);
    phongShader->setMat4("modelMtx", glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -3.0f, 0.0f)), glm::vec3(15.0f, 0.2f, 15.0f)));
    cube1.draw();
}

void Simulator::processInput(GLFWwindow* window, float deltaTime) {
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
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
        moveDirection.y -= 1.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        moveDirection.y += 1.0f;
    }
    if (moveDirection != glm::vec3(0.0f, 0.0f, 0.0f)) {
        camera.processKeyboard(glm::normalize(moveDirection), deltaTime);
    }
}
