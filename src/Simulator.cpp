#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Mesh.h"
#include "Model.h"
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
unique_ptr<Framebuffer> Simulator::fbo;

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
        
        unsigned int blackTexture = generateTexture(0, 0, 0);
        unsigned int diffuseMap = loadTexture("textures/container2.png");
        unsigned int specularMap = loadTexture("textures/container2_specular.png");
        
        Shader skyboxShader("shaders/skyboxShader.v.glsl", "shaders/skyboxShader.f.glsl");
        Shader lightShader("shaders/phongShader.v.glsl", "shaders/lightShader.f.glsl");
        Shader phongShader("shaders/phongShader.v.glsl", "shaders/phongShader.f.glsl");
        Shader framebufferShader("shaders/framebufferShader.v.glsl", "shaders/framebufferShader.f.glsl");
        Shader testShader("shaders/phongShader.v.glsl", "shaders/blendShader.f.glsl");
        
        const int NUM_LIGHTS = 8;
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
        Mesh lightCube;
        lightCube.generateCube(0.2f);
        
        Mesh cube1;
        cube1.generateCube();
        //stbi_set_flip_vertically_on_load(false);
        Model modelTest("models/nanosuit/nanosuit.obj");
        //stbi_set_flip_vertically_on_load(true);
        
        vector<Mesh::Vertex> grassVertices = {
            { 0.5f,  1.0f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f},
            {-0.5f,  1.0f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f},
            {-0.5f,  0.0f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f},
            { 0.5f,  0.0f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f}
        };
        vector<unsigned int> grassIndices = {
            0, 1, 2, 2, 3, 0
        };
        unsigned int grassDiffuse = loadTexture("textures/blending_transparent_window.png");
        Mesh grass(move(grassVertices), move(grassIndices));
        
        vector<glm::vec3> grassPositions = {
            {-1.5f,  0.0f, -0.5f},
            { 1.5f,  0.0f,  0.5f},
            { 0.0f,  0.0f,  0.7f},
            {-0.3f,  0.0f, -2.3f},
            { 0.5f,  0.0f, -0.6f}
        };
        
        vector<Mesh::Vertex> windowQuadVertices = {
            { 1.0f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f, 1.0f, 1.0f},
            {-1.0f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f, 0.0f, 1.0f},
            {-1.0f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f, 0.0f, 0.0f},
            { 1.0f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f, 1.0f, 0.0f}
        };
        vector<unsigned int> windowQuadIndices = {
            0, 1, 2, 2, 3, 0
        };
        Mesh windowQuad(move(windowQuadVertices), move(windowQuadIndices));
        
        unsigned int skyboxTexture = loadCubemap("textures/skybox/.jpg");
        vector<Mesh::Vertex> skyboxVertices = {
            {-1.0f,  1.0f, -1.0f, 0.0, 0.0, 0.0, 0.0, 0.0},
            {-1.0f, -1.0f, -1.0f, 0.0, 0.0, 0.0, 0.0, 0.0},
            { 1.0f, -1.0f, -1.0f, 0.0, 0.0, 0.0, 0.0, 0.0},
            { 1.0f, -1.0f, -1.0f, 0.0, 0.0, 0.0, 0.0, 0.0},
            { 1.0f,  1.0f, -1.0f, 0.0, 0.0, 0.0, 0.0, 0.0},
            {-1.0f,  1.0f, -1.0f, 0.0, 0.0, 0.0, 0.0, 0.0},

            {-1.0f, -1.0f,  1.0f, 0.0, 0.0, 0.0, 0.0, 0.0},
            {-1.0f, -1.0f, -1.0f, 0.0, 0.0, 0.0, 0.0, 0.0},
            {-1.0f,  1.0f, -1.0f, 0.0, 0.0, 0.0, 0.0, 0.0},
            {-1.0f,  1.0f, -1.0f, 0.0, 0.0, 0.0, 0.0, 0.0},
            {-1.0f,  1.0f,  1.0f, 0.0, 0.0, 0.0, 0.0, 0.0},
            {-1.0f, -1.0f,  1.0f, 0.0, 0.0, 0.0, 0.0, 0.0},

            { 1.0f, -1.0f, -1.0f, 0.0, 0.0, 0.0, 0.0, 0.0},
            { 1.0f, -1.0f,  1.0f, 0.0, 0.0, 0.0, 0.0, 0.0},
            { 1.0f,  1.0f,  1.0f, 0.0, 0.0, 0.0, 0.0, 0.0},
            { 1.0f,  1.0f,  1.0f, 0.0, 0.0, 0.0, 0.0, 0.0},
            { 1.0f,  1.0f, -1.0f, 0.0, 0.0, 0.0, 0.0, 0.0},
            { 1.0f, -1.0f, -1.0f, 0.0, 0.0, 0.0, 0.0, 0.0},

            {-1.0f, -1.0f,  1.0f, 0.0, 0.0, 0.0, 0.0, 0.0},
            {-1.0f,  1.0f,  1.0f, 0.0, 0.0, 0.0, 0.0, 0.0},
            { 1.0f,  1.0f,  1.0f, 0.0, 0.0, 0.0, 0.0, 0.0},
            { 1.0f,  1.0f,  1.0f, 0.0, 0.0, 0.0, 0.0, 0.0},
            { 1.0f, -1.0f,  1.0f, 0.0, 0.0, 0.0, 0.0, 0.0},
            {-1.0f, -1.0f,  1.0f, 0.0, 0.0, 0.0, 0.0, 0.0},

            {-1.0f,  1.0f, -1.0f, 0.0, 0.0, 0.0, 0.0, 0.0},
            { 1.0f,  1.0f, -1.0f, 0.0, 0.0, 0.0, 0.0, 0.0},
            { 1.0f,  1.0f,  1.0f, 0.0, 0.0, 0.0, 0.0, 0.0},
            { 1.0f,  1.0f,  1.0f, 0.0, 0.0, 0.0, 0.0, 0.0},
            {-1.0f,  1.0f,  1.0f, 0.0, 0.0, 0.0, 0.0, 0.0},
            {-1.0f,  1.0f, -1.0f, 0.0, 0.0, 0.0, 0.0, 0.0},

            {-1.0f, -1.0f, -1.0f, 0.0, 0.0, 0.0, 0.0, 0.0},
            {-1.0f, -1.0f,  1.0f, 0.0, 0.0, 0.0, 0.0, 0.0},
            { 1.0f, -1.0f, -1.0f, 0.0, 0.0, 0.0, 0.0, 0.0},
            { 1.0f, -1.0f, -1.0f, 0.0, 0.0, 0.0, 0.0, 0.0},
            {-1.0f, -1.0f,  1.0f, 0.0, 0.0, 0.0, 0.0, 0.0},
            { 1.0f, -1.0f,  1.0f, 0.0, 0.0, 0.0, 0.0, 0.0}
        };
        vector<unsigned int> skyboxIndices;
        for (unsigned int i = 0; i < skyboxVertices.size(); ++i) {
            skyboxIndices.push_back(i);
        }
        Mesh skybox(move(skyboxVertices), move(skyboxIndices));
        //skybox.generateCube();
        
        fbo = make_unique<Framebuffer>(windowSize);
        
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        
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
        for (int i = 0; i < 5000; ++i) {
            //cubePositions.emplace_back(randomFloat(-50.0f, 50.0f), randomFloat(-50.0f, 50.0f), randomFloat(-50.0f, 50.0f));
        }
        
        double lastTime = glfwGetTime();
        float deltaTime = 0.0f;
        double lastFrameTime = lastTime;
        int frameCounter = 0;
        cout << "Setup complete.\n";
        while (state != State::Exiting) {    // Render loop.
            double currentTime = glfwGetTime();
            deltaTime = static_cast<float>(currentTime - lastTime);
            lastTime = currentTime;
            
            /*++frameCounter;
            if (currentTime - lastFrameTime >= 1.0) {
                cout << 1000.0 / frameCounter << " ms/frame\n";
                frameCounter = 0;
                lastFrameTime += 1.0;
                if (currentTime - lastFrameTime >= 1.0) {
                    lastFrameTime = currentTime;
                }
            }*/
            
            processInput(window, deltaTime);
            
            fbo->bind();
            glViewport(0, 0, fbo->getBufferSize().x, fbo->getBufferSize().y);
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
            
            for (int i = 0; i < 4; ++i) {
                lightShader.setVec3("lightColor", pointLightColors[i]);
                lightShader.setMat4("modelMtx", glm::translate(glm::mat4(1.0f), pointLightPositions[i]));
                lightCube.draw();
            }
            
            phongShader.use();
            phongShader.setMat4("viewMtx", viewMtx);
            phongShader.setMat4("projectionMtx", projectionMtx);
            phongShader.setInt("material.texDiffuse0", 0);
            phongShader.setInt("material.texSpecular0", 1);
            phongShader.setFloat("material.shininess", 32.0f);
            
            unsigned int lightStates[NUM_LIGHTS] = {1, 1, 1, 1, 1, 1, 0, 0};
            phongShader.setUnsignedIntArray("lightStates", NUM_LIGHTS, lightStates);
            
            phongShader.setUnsignedInt("lights[0].type", 0);
            phongShader.setVec3("lights[0].directionViewSpace", viewMtx * glm::vec4(-0.2f, -1.0f, -0.3f, 0.0f));
            glm::vec3 directionalLightColor(1.0f, 1.0f, 1.0f);
            phongShader.setVec3("lights[0].ambient", directionalLightColor * 0.05f);
            phongShader.setVec3("lights[0].diffuse", directionalLightColor * 0.4f);
            phongShader.setVec3("lights[0].specular", directionalLightColor * 0.5f);
            
            for (int i = 0; i < 4; ++i) {
                phongShader.setUnsignedInt("lights[" + to_string(i + 1) + "].type", 1);
                phongShader.setVec3("lights[" + to_string(i + 1) + "].positionViewSpace", viewMtx * glm::vec4(pointLightPositions[i], 1.0f));
                phongShader.setVec3("lights[" + to_string(i + 1) + "].ambient", pointLightColors[i] * 0.05f);
                phongShader.setVec3("lights[" + to_string(i + 1) + "].diffuse", pointLightColors[i] * 0.8f);
                phongShader.setVec3("lights[" + to_string(i + 1) + "].specular", pointLightColors[i]);
                phongShader.setVec3("lights[" + to_string(i + 1) + "].attenuationVals", glm::vec3(1.0f, 0.09f, 0.032f));
            }
            
            phongShader.setUnsignedInt("lights[5].type", 2);
            phongShader.setVec3("lights[5].positionViewSpace", viewMtx * glm::vec4(camera.position, 1.0f));
            phongShader.setVec3("lights[5].directionViewSpace", viewMtx * glm::vec4(camera.front, 0.0f));
            glm::vec3 spotLightColor(1.0f, 1.0f, 1.0f);
            phongShader.setVec3("lights[5].ambient", spotLightColor * 0.0f);
            phongShader.setVec3("lights[5].diffuse", spotLightColor);
            phongShader.setVec3("lights[5].specular", spotLightColor);
            phongShader.setVec3("lights[5].attenuationVals", glm::vec3(1.0f, 0.09f, 0.032f));
            phongShader.setVec2("lights[5].cutOff", glm::vec2(glm::cos(glm::radians(12.5f)), glm::cos(glm::radians(17.5f))));
            
            phongShader.setMat4("modelMtx", glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)), glm::vec3(0.1f, 0.1f, 0.1f)));
            modelTest.draw(phongShader);
            
            skyboxShader.use();    // Issue with skybox currently where it is inverted. #######################################################################################
            glDepthFunc(GL_LEQUAL);
            glDisable(GL_CULL_FACE);
            skyboxShader.setMat4("viewMtx", glm::mat4(glm::mat3(viewMtx)));
            skyboxShader.setMat4("projectionMtx", projectionMtx);
            skyboxShader.setInt("skybox", 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
            skybox.draw();
            glDepthFunc(GL_LESS);
            glEnable(GL_CULL_FACE);
            
            testShader.use();
            glEnable(GL_BLEND);
            testShader.setMat4("viewMtx", viewMtx);
            testShader.setMat4("projectionMtx", projectionMtx);
            testShader.setInt("material.texDiffuse0", 0);
            
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, diffuseMap);
            //glActiveTexture(GL_TEXTURE1);
            //glBindTexture(GL_TEXTURE_2D, specularMap);
            for (unsigned int i = 0; i < cubePositions.size(); ++i) {
                glm::mat4 modelMtx = glm::translate(glm::mat4(1.0f), cubePositions[i]);
                float angle = 20.0f * i;
                modelMtx = glm::rotate(modelMtx, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
                testShader.setMat4("modelMtx", modelMtx);
                cube1.draw();
            }
            testShader.setMat4("modelMtx", glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, -0.5f, 3.0f)), glm::vec3(3.0f, 0.2f, 5.0f)));
            cube1.draw();
            
            glDepthMask(false);
            glDisable(GL_CULL_FACE);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, grassDiffuse);
            for (unsigned int i = 0; i < grassPositions.size(); ++i) {
                testShader.setMat4("modelMtx", glm::translate(glm::mat4(1.0f), grassPositions[i]));
                grass.draw();
            }
            glDepthMask(true);
            glEnable(GL_CULL_FACE);
            glDisable(GL_BLEND);
            
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(0, 0, windowSize.x, windowSize.y);
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            glDisable(GL_DEPTH_TEST);
            framebufferShader.use();
            framebufferShader.setInt("tex", 0);
            fbo->bindTexColorBuffer();
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
    
    fbo.reset();
    
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
    int width, height, numChannels;
    unsigned char* imageData = stbi_load(filename.c_str(), &width, &height, &numChannels, 0);
    GLenum format = numChannels == 3 ? GL_RGB : GL_RGBA;
    
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
    
    stbi_set_flip_vertically_on_load(false);
    string prefix = filename.substr(0, filename.find('.'));
    string postfix = filename.substr(filename.find('.'));
    cout << "Loading cubemap.\n";
    unsigned int texHandle;
    glGenTextures(1, &texHandle);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texHandle);
    
    int width, height, numChannels;
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
        GLenum format = numChannels == 3 ? GL_RGB : GL_RGBA;
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
    fbo->setBufferSize(windowSize);
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
