#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Font.h"
#include "Framebuffer.h"
#include "PerformanceMonitor.h"
#include "Shader.h"
#include "Simulator.h"
#include <cassert>
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <utility>

atomic<Simulator::State> Simulator::state = {State::Uninitialized};
mt19937 Simulator::mainRNG;
Camera Simulator::camera(glm::vec3(0.0f, 1.8f, 2.0f));
Configuration Simulator::config;
glm::ivec2 Simulator::windowSize(800, 600);
glm::vec2 Simulator::lastMousePos(windowSize.x / 2.0f, windowSize.y / 2.0f);
unordered_map<string, unsigned int> Simulator::loadedTextures;
unique_ptr<Shader> Simulator::geometryShader, Simulator::geometryNormalMapShader, Simulator::geometrySkinningShader, Simulator::lightingPassShader, Simulator::skyboxShader, Simulator::lampShader, Simulator::shadowMapShader, Simulator::shadowMapSkinningShader, Simulator::textShader, Simulator::shapeShader;
unique_ptr<Shader> Simulator::postProcessShader, Simulator::bloomShader, Simulator::gaussianBlurShader, Simulator::ssaoShader, Simulator::ssaoBlurShader;
unique_ptr<Framebuffer> Simulator::geometryFBO, Simulator::renderFBO, Simulator::cascadedShadowFBO[NUM_CASCADED_SHADOWS];
unique_ptr<Framebuffer> Simulator::bloom1FBO, Simulator::bloom2FBO, Simulator::ssaoFBO, Simulator::ssaoBlurFBO;
unsigned int Simulator::blackTexture, Simulator::whiteTexture, Simulator::blueTexture, Simulator::cubeDiffuseMap, Simulator::cubeSpecularMap, Simulator::woodTexture, Simulator::skyboxCubemap, Simulator::brickDiffuseMap, Simulator::brickNormalMap, Simulator::ssaoNoiseTexture, Simulator::monitorGridTexture;
unsigned int Simulator::viewProjectionMtxUBO;
Mesh Simulator::lightCube, Simulator::cube1, Simulator::sphere1, Simulator::windowQuad, Simulator::skybox;
ModelStatic Simulator::sceneTest;
ModelRigged Simulator::modelTest;
Transformable Simulator::modelTestTransform, Simulator::sceneTestTransform;
bool Simulator::flashlightOn = false, Simulator::sunlightOn = true, Simulator::lampsOn = false, Simulator::test;
float Simulator::sunT = 0.0f, Simulator::sunSpeed = 1.0f;

int Simulator::start() {
    cout << "Initializing setup...\n";
    int exitCode = 0;
    GLFWwindow* window = nullptr;
    try {
        assert(state == State::Uninitialized);
        state = State::Running;
        mainRNG.seed(static_cast<unsigned long>(chrono::high_resolution_clock::now().time_since_epoch().count()));
        
        window = setupOpenGL();
        setupTextures();
        setupShaders();
        setupBuffers();
        setupSimulation();
        
        config.setVsync(true);
        config.setBloom(true);
        config.setSSAO(true);
        
        shared_ptr<Font> arialFont = make_shared<Font>();    // why not combine these two, no reason to reset a font. #####################################################################################
        arialFont->loadFont("fonts/arial.ttf", 15);
        
        PerformanceMonitor frameMonitor("FRAME", arialFont);
        frameMonitor.modelMtx_ = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
        PerformanceMonitor ssaoMonitor("SSAO", arialFont);
        ssaoMonitor.modelMtx_ = glm::translate(glm::mat4(1.0f), glm::vec3(220.0f, 0.0f, 0.0f));
        PerformanceMonitor bloomMonitor("BLOOM", arialFont);
        bloomMonitor.modelMtx_ = glm::translate(glm::mat4(1.0f), glm::vec3(440.0f, 0.0f, 0.0f));
        
        vector<glm::mat4> boneTransforms(128);
        for (size_t i = 0; i < boneTransforms.size(); ++i) {
            boneTransforms[i] = glm::mat4(1.0f);
        }
        
        geometrySkinningShader->use();
        geometrySkinningShader->setMat4Array("boneTransforms", static_cast<unsigned int>(boneTransforms.size()), boneTransforms.data());
        shadowMapSkinningShader->use();
        shadowMapSkinningShader->setMat4Array("boneTransforms", static_cast<unsigned int>(boneTransforms.size()), boneTransforms.data());
        
        constexpr float SHADOW_BOUND_CORRECTION = 0.8f;    // Split points are exponentially distributed with a linear term. https://developer.download.nvidia.com/SDK/10.5/opengl/src/cascaded_shadow_maps/doc/cascaded_shadow_maps.pdf
        float shadowZBounds[NUM_CASCADED_SHADOWS + 1];
        shadowZBounds[0] = NEAR_PLANE;
        for (unsigned int i = 1; i < NUM_CASCADED_SHADOWS; ++i) {
            shadowZBounds[i] = SHADOW_BOUND_CORRECTION * NEAR_PLANE * pow(FAR_PLANE / NEAR_PLANE, static_cast<float>(i) / NUM_CASCADED_SHADOWS) + (1.0f - SHADOW_BOUND_CORRECTION) * (NEAR_PLANE + static_cast<float>(i) / NUM_CASCADED_SHADOWS) * (FAR_PLANE - NEAR_PLANE);
        }
        shadowZBounds[NUM_CASCADED_SHADOWS] = FAR_PLANE;
        
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
            
            frameMonitor.startGPUTimer();
            
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
            sunT += sunSpeed * deltaTime;
            
            modelTest.animate(0, currentTime, boneTransforms);
            geometrySkinningShader->use();
            geometrySkinningShader->setMat4Array("boneTransforms", static_cast<unsigned int>(boneTransforms.size()), boneTransforms.data());
            shadowMapSkinningShader->use();
            shadowMapSkinningShader->setMat4Array("boneTransforms", static_cast<unsigned int>(boneTransforms.size()), boneTransforms.data());
            
            constexpr int NUM_LIGHTS = 8;
            glm::vec3 pointLightPositions[4] = {
                {0.7f, 0.2f, 2.0f},
                {2.3f, -3.3f, -4.0f},
                {-4.0f, 2.0f, -12.0f},
                {0.0f, 0.0f, -3.0f}
            };
            glm::vec3 pointLightColors[4] = {
                {5.0f, 5.0f, 5.0f},
                {10.0f, 0.0f, 0.0f},
                {0.0f, 0.0f, 15.0f},
                {0.0f, 5.0f, 0.0f}
            };
            unsigned int lightStates[NUM_LIGHTS] = {0, 0, 0, 0, 0, 0, 0, 0};
            lightStates[0] = (sunlightOn ? 1u : 0u);
            lightStates[1] = (flashlightOn ? 1u : 0u);
            for (int i = 0; i < 4; ++i) {
                lightStates[i + 2] = (lampsOn ? 1u : 0u);
            }
            glm::vec3 sunPosition = glm::vec3(glm::rotate(glm::mat4(1.0f), sunT, glm::vec3(1.0f, 1.0f, 1.0f)) * glm::vec4(0.0f, 0.0f, FAR_PLANE, 1.0f));
            if (sunPosition.x == 0.0f && sunPosition.z == 0.0f) {    // Fix edge case when directional light aligns with up vector.
                sunPosition.x = 0.00001f;
            }
            
            glm::mat4 shadowProjections[NUM_CASCADED_SHADOWS];
            glm::vec2 tanHalfFOV(tan(glm::radians(camera.fov_ / 2.0f)) * (static_cast<float>(windowSize.x) / windowSize.y), tan(glm::radians(camera.fov_ / 2.0f)));
            glm::mat4 lightViewMtx = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), -sunPosition, glm::vec3(0.0f, 1.0f, 0.0f));
            glm::mat4 viewToLightSpace = lightViewMtx * glm::inverse(camera.getViewMatrix());
            
            for (unsigned int i = 0; i < NUM_CASCADED_SHADOWS; ++i) {    // Calculate an orthographic projection for each of the cascaded shadow volumes.
                float xNear = shadowZBounds[i] * tanHalfFOV.x;
                float xFar = shadowZBounds[i + 1] * tanHalfFOV.x;
                float yNear = shadowZBounds[i] * tanHalfFOV.y;
                float yFar = shadowZBounds[i + 1] * tanHalfFOV.y;
                
                glm::vec4 frustumCorners[8] = {
                    { xNear,  yNear, -shadowZBounds[i], 1.0f},
                    {-xNear,  yNear, -shadowZBounds[i], 1.0f},
                    { xNear, -yNear, -shadowZBounds[i], 1.0f},
                    {-xNear, -yNear, -shadowZBounds[i], 1.0f},
                    
                    { xFar,  yFar, -shadowZBounds[i + 1], 1.0f},
                    {-xFar,  yFar, -shadowZBounds[i + 1], 1.0f},
                    { xFar, -yFar, -shadowZBounds[i + 1], 1.0f},
                    {-xFar, -yFar, -shadowZBounds[i + 1], 1.0f}
                };
                
                glm::vec3 minBound(numeric_limits<float>::max());
                glm::vec3 maxBound(numeric_limits<float>::lowest());
                for (unsigned int j = 0; j < 8; ++j) {
                    frustumCorners[j] = viewToLightSpace * frustumCorners[j];    // Convert the frustum corner to light space.
                    
                    minBound.x = min(minBound.x, frustumCorners[j].x);
                    minBound.y = min(minBound.y, frustumCorners[j].y);
                    minBound.z = min(minBound.z, frustumCorners[j].z);
                    
                    maxBound.x = max(maxBound.x, frustumCorners[j].x);
                    maxBound.y = max(maxBound.y, frustumCorners[j].y);
                    maxBound.z = max(maxBound.z, frustumCorners[j].z);
                }
                
                constexpr float NEAR_PLANE_PADDING = FAR_PLANE;    // Extra padding added to near plane to extend the shadow volume behind the camera.
                shadowProjections[i] = glm::ortho(minBound.x, maxBound.x, minBound.y, maxBound.y, -maxBound.z - NEAR_PLANE_PADDING, -minBound.z);
            }
            
            glViewport(0, 0, cascadedShadowFBO[0]->getBufferSize().x, cascadedShadowFBO[0]->getBufferSize().y);
            glCullFace(GL_FRONT);
            for (unsigned int i = 0; i < NUM_CASCADED_SHADOWS; ++i) {    // Render to cascaded shadow maps.
                cascadedShadowFBO[i]->bind();
                glClear(GL_DEPTH_BUFFER_BIT);
                renderScene(lightViewMtx, shadowProjections[i], true);
            }
            glCullFace(GL_BACK);
            
            
            geometryFBO->bind();    // Render to geometry buffer (geometry pass).
            glViewport(0, 0, geometryFBO->getBufferSize().x, geometryFBO->getBufferSize().y);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glm::mat4 viewMtx = camera.getViewMatrix();
            glm::mat4 projectionMtx = glm::perspective(glm::radians(camera.fov_), static_cast<float>(windowSize.x) / windowSize.y, NEAR_PLANE, FAR_PLANE);
            renderScene(viewMtx, projectionMtx, false);
            
            geometryFBO->bind(GL_READ_FRAMEBUFFER);    // Copy depth buffer over.
            renderFBO->bind(GL_DRAW_FRAMEBUFFER);
            glBlitFramebuffer(0, 0, geometryFBO->getBufferSize().x, geometryFBO->getBufferSize().y, 0, 0, renderFBO->getBufferSize().x, renderFBO->getBufferSize().y, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
            
            ssaoMonitor.startGPUTimer();
            if (config.getSSAO()) {
                ssaoFBO->bind();    // Render SSAO texture.
                glViewport(0, 0, ssaoFBO->getBufferSize().x, ssaoFBO->getBufferSize().y);
                glClear(GL_COLOR_BUFFER_BIT);
                ssaoShader->use();
                ssaoShader->setInt("texPosition", 0);
                ssaoShader->setInt("texNormal", 1);
                ssaoShader->setInt("texNoise", 2);
                ssaoShader->setVec2("noiseScale", glm::vec2(ssaoFBO->getBufferSize().x / 4.0f, ssaoFBO->getBufferSize().y / 4.0f));
                glActiveTexture(GL_TEXTURE0);
                geometryFBO->bindTexture(0);
                glActiveTexture(GL_TEXTURE1);
                geometryFBO->bindTexture(1);
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, ssaoNoiseTexture);
                windowQuad.drawGeometry();
                
                ssaoBlurFBO->bind();    // Blur SSAO texture.
                glViewport(0, 0, ssaoBlurFBO->getBufferSize().x, ssaoBlurFBO->getBufferSize().y);
                glClear(GL_COLOR_BUFFER_BIT);
                ssaoBlurShader->use();
                ssaoBlurShader->setInt("image", 0);
                glActiveTexture(GL_TEXTURE0);
                ssaoFBO->bindTexture(0);
                windowQuad.drawGeometry();
            }
            ssaoMonitor.stopGPUTimer();
            
            renderFBO->bind();    // Render lighting (lighting pass).
            glViewport(0, 0, renderFBO->getBufferSize().x, renderFBO->getBufferSize().y);
            glClear(GL_COLOR_BUFFER_BIT);
            glDisable(GL_DEPTH_TEST);
            lightingPassShader->use();
            lightingPassShader->setInt("texPosition", 0);
            glActiveTexture(GL_TEXTURE0);
            geometryFBO->bindTexture(0);
            lightingPassShader->setInt("texNormal", 1);
            glActiveTexture(GL_TEXTURE1);
            geometryFBO->bindTexture(1);
            lightingPassShader->setInt("texAlbedoSpec", 2);
            glActiveTexture(GL_TEXTURE2);
            geometryFBO->bindTexture(2);
            lightingPassShader->setInt("texSSAO", 3);
            if (config.getSSAO()) {
                glActiveTexture(GL_TEXTURE3);
                ssaoBlurFBO->bindTexture(0);
            }
            for (unsigned int i = 0; i < NUM_CASCADED_SHADOWS; ++i) {
                lightingPassShader->setInt("shadowMap[" + to_string(i) + "]", 4 + i);
                glActiveTexture(GL_TEXTURE4 + i);
                cascadedShadowFBO[i]->bindTexture(0);
                lightingPassShader->setMat4("viewToLightSpace[" + to_string(i) + "]", shadowProjections[i] * viewToLightSpace);
                lightingPassShader->setFloat("shadowZEnds[" + to_string(i) + "]", shadowZBounds[i + 1]);
            }
            lightingPassShader->setBool("applySSAO", config.getSSAO());
            lightingPassShader->setUnsignedIntArray("lightStates", NUM_LIGHTS, lightStates);
            lightingPassShader->setUnsignedInt("lights[0].type", 0);
            lightingPassShader->setVec3("lights[0].directionViewSpace", viewMtx * glm::vec4(-sunPosition, 0.0f));
            glm::vec3 directionalLightColor(1.0f, 1.0f, 1.0f);
            lightingPassShader->setVec3("lights[0].ambient", directionalLightColor * 0.05f);
            lightingPassShader->setVec3("lights[0].diffuse", directionalLightColor * 0.4f);
            lightingPassShader->setVec3("lights[0].specular", directionalLightColor * 0.5f);
            lightingPassShader->setUnsignedInt("lights[1].type", 2);
            lightingPassShader->setVec3("lights[1].positionViewSpace", viewMtx * glm::vec4(camera.position_, 1.0f));
            lightingPassShader->setVec3("lights[1].directionViewSpace", viewMtx * glm::vec4(camera.front_, 0.0f));
            glm::vec3 spotLightColor(1.0f, 1.0f, 1.0f);
            lightingPassShader->setVec3("lights[1].ambient", spotLightColor * 0.0f);
            lightingPassShader->setVec3("lights[1].diffuse", spotLightColor);
            lightingPassShader->setVec3("lights[1].specular", spotLightColor);
            lightingPassShader->setVec3("lights[1].attenuationVals", glm::vec3(1.0f, 0.09f, 0.032f));
            lightingPassShader->setVec2("lights[1].cutOff", glm::vec2(glm::cos(glm::radians(12.5f)), glm::cos(glm::radians(17.5f))));
            for (int i = 0; i < 4; ++i) {
                lightingPassShader->setUnsignedInt("lights[" + to_string(i + 2) + "].type", 1);
                lightingPassShader->setVec3("lights[" + to_string(i + 2) + "].positionViewSpace", viewMtx * glm::vec4(pointLightPositions[i], 1.0f));
                lightingPassShader->setVec3("lights[" + to_string(i + 2) + "].ambient", pointLightColors[i] * 0.05f);
                lightingPassShader->setVec3("lights[" + to_string(i + 2) + "].diffuse", pointLightColors[i] * 0.8f);
                lightingPassShader->setVec3("lights[" + to_string(i + 2) + "].specular", pointLightColors[i]);
                lightingPassShader->setVec3("lights[" + to_string(i + 2) + "].attenuationVals", glm::vec3(1.0f, 0.09f, 0.032f));
            }
            windowQuad.drawGeometry();
            glEnable(GL_DEPTH_TEST);
            
            lampShader->use();    // Draw lamps.
            for (int i = 0; i < 4; ++i) {
                if (lightStates[i + 2] == 1) {
                    lampShader->setVec3("lightColor", pointLightColors[i]);
                    lightCube.drawGeometry(*lampShader, glm::translate(glm::mat4(1.0f), pointLightPositions[i]));
                }
            }
            lampShader->setVec3("lightColor", glm::vec3(1.0f, 1.0f, 0.0f));
            lightCube.drawGeometry(*lampShader, glm::scale(glm::translate(glm::mat4(1.0f), sunPosition + camera.position_), glm::vec3(5.0f)));
            
            skyboxShader->use();    // Draw the skybox.
            glDepthFunc(GL_LEQUAL);
            glDisable(GL_CULL_FACE);
            skyboxShader->setInt("skybox", 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxCubemap);
            skybox.drawGeometry();
            glDepthFunc(GL_LESS);
            glEnable(GL_CULL_FACE);
            
            bloomMonitor.startGPUTimer();
            if (config.getBloom()) {
                bloom1FBO->bind();    // Compute bloom texture.
                glViewport(0, 0, bloom1FBO->getBufferSize().x, bloom1FBO->getBufferSize().y);
                glClear(GL_COLOR_BUFFER_BIT);
                bloomShader->use();
                bloomShader->setInt("image", 0);
                glActiveTexture(GL_TEXTURE0);
                renderFBO->bindTexture(0);
                windowQuad.drawGeometry();
                
                gaussianBlurShader->use();    // Apply Gaussian blur to texture.
                gaussianBlurShader->setInt("image", 0);
                glActiveTexture(GL_TEXTURE0);
                for (unsigned int i = 0; i < 5; ++i) {
                    bloom2FBO->bind();    // Draw to bloom 2 framebuffer.
                    gaussianBlurShader->setBool("blurHorizontal", true);
                    bloom1FBO->bindTexture(0);
                    windowQuad.drawGeometry();
                    
                    bloom1FBO->bind();    // Draw to bloom 1 framebuffer.
                    gaussianBlurShader->setBool("blurHorizontal", false);
                    bloom2FBO->bindTexture(0);
                    windowQuad.drawGeometry();
                }
            }
            bloomMonitor.stopGPUTimer();
            
            glBindFramebuffer(GL_FRAMEBUFFER, 0);    // Apply post-processing and render to window.
            glViewport(0, 0, windowSize.x, windowSize.y);
            glClear(GL_COLOR_BUFFER_BIT);
            glDisable(GL_DEPTH_TEST);
            postProcessShader->use();
            postProcessShader->setInt("image", 0);
            postProcessShader->setInt("bloomBlur", 1);
            postProcessShader->setFloat("exposure", 1.0f);
            postProcessShader->setBool("applyBloom", config.getBloom());
            glActiveTexture(GL_TEXTURE0);
            renderFBO->bindTexture(0);
            if (config.getBloom()) {
                glActiveTexture(GL_TEXTURE1);
                bloom1FBO->bindTexture(0);
            }
            windowQuad.drawGeometry();
            glEnable(GL_DEPTH_TEST);
            
            glEnable(GL_BLEND);    // Render GUI.
            glDisable(GL_DEPTH_TEST);
            glm::mat4 windowProjectionMtx = glm::ortho(0.0f, static_cast<float>(windowSize.x), 0.0f, static_cast<float>(windowSize.y));
            shapeShader->use();
            textShader->setMat4("projectionMtx", windowProjectionMtx);
            shapeShader->setInt("tex", 0);
            shapeShader->setVec4("color", glm::vec4(1.0f, 1.0f, 1.0f, 0.7f));
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, monitorGridTexture);
            frameMonitor.drawBox(*shapeShader, glm::mat4(1.0f));
            ssaoMonitor.drawBox(*shapeShader, glm::mat4(1.0f));
            bloomMonitor.drawBox(*shapeShader, glm::mat4(1.0f));
            shapeShader->setVec4("color", glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
            glBindTexture(GL_TEXTURE_2D, whiteTexture);
            frameMonitor.drawLine(*shapeShader, glm::mat4(1.0f));
            ssaoMonitor.drawLine(*shapeShader, glm::mat4(1.0f));
            bloomMonitor.drawLine(*shapeShader, glm::mat4(1.0f));
            textShader->use();
            textShader->setMat4("projectionMtx", windowProjectionMtx);
            textShader->setInt("texFont", 0);
            textShader->setVec3("color", glm::vec3(0.8f, 0.8f, 0.8f));
            frameMonitor.drawText(*textShader, glm::mat4(1.0f));
            ssaoMonitor.drawText(*textShader, glm::mat4(1.0f));
            bloomMonitor.drawText(*textShader, glm::mat4(1.0f));
            glDisable(GL_BLEND);
            glEnable(GL_DEPTH_TEST);
            
            frameMonitor.stopGPUTimer();
            
            frameMonitor.update();    // Monitor update must occur after drawing.
            ssaoMonitor.update();
            bloomMonitor.update();
            
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
    
    glDeleteBuffers(1, &viewProjectionMtxUBO);    // Clean up allocated resources.
    geometryShader.reset();
    geometryNormalMapShader.reset();
    geometrySkinningShader.reset();
    lightingPassShader.reset();
    skyboxShader.reset();
    lampShader.reset();
    shadowMapShader.reset();
    shadowMapSkinningShader.reset();
    textShader.reset();
    shapeShader.reset();
    postProcessShader.reset();
    bloomShader.reset();
    gaussianBlurShader.reset();
    ssaoShader.reset();
    ssaoBlurShader.reset();
    
    geometryFBO.reset();
    renderFBO.reset();
    for (unsigned int i = 0; i < NUM_CASCADED_SHADOWS; ++i) {
        cascadedShadowFBO[i].reset();
    }
    bloom1FBO.reset();
    bloom2FBO.reset();
    ssaoFBO.reset();
    ssaoBlurFBO.reset();
    
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

unsigned int Simulator::loadTexture(const string& filename, bool gammaCorrection, bool flip) {
    string textureName = filename + (gammaCorrection ? "-g" : "") + (flip ? "-f" : "");
    auto findResult = loadedTextures.find(textureName);
    if (findResult != loadedTextures.end()) {
        return findResult->second;
    }
    
    stbi_set_flip_vertically_on_load(flip);
    cout << "Loading texture \"" << textureName << "\".\n";
    unsigned int texHandle;
    glGenTextures(1, &texHandle);
    glBindTexture(GL_TEXTURE_2D, texHandle);
    int width, height, numChannels = 1;
    unsigned char* imageData = stbi_load(filename.c_str(), &width, &height, &numChannels, 0);
    GLenum internalFormat, format;
    if (numChannels == 1) {
        internalFormat = GL_RED;
        format = GL_RED;
    } else if (numChannels == 3) {
        internalFormat = (gammaCorrection ? GL_SRGB : GL_RGB);
        format = GL_RGB;
    } else if (numChannels == 4) {
        internalFormat = (gammaCorrection ? GL_SRGB_ALPHA : GL_RGBA);
        format = GL_RGBA;
    } else {
        cout << "Error: Unsupported channel number.\n";
        stbi_image_free(imageData);
        imageData = nullptr;
    }
    
    if (imageData) {
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, imageData);
        
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    } else {
        cout << "Error: Unable to load texture.\n";
    }
    stbi_image_free(imageData);
    
    loadedTextures[textureName] = texHandle;
    return texHandle;
}

unsigned int Simulator::loadCubemap(const string& filename, bool gammaCorrection, bool flip) {
    string textureName = filename + (gammaCorrection ? "-g" : "") + (flip ? "-f" : "");
    auto findResult = loadedTextures.find(textureName);
    if (findResult != loadedTextures.end()) {
        return findResult->second;
    }
    
    stbi_set_flip_vertically_on_load(flip);    // For skybox cubemap, textures are not flipped to match the specifications of a cubemap.
    string prefix = filename.substr(0, filename.find('.'));
    string postfix = filename.substr(filename.find('.'));
    cout << "Loading cubemap \"" << textureName << "\".\n";
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
        GLenum internalFormat, format;
        if (numChannels == 1) {
            internalFormat = GL_RED;
            format = GL_RED;
        } else if (numChannels == 3) {
            internalFormat = (gammaCorrection ? GL_SRGB : GL_RGB);
            format = GL_RGB;
        } else if (numChannels == 4) {
            internalFormat = (gammaCorrection ? GL_SRGB_ALPHA : GL_RGBA);
            format = GL_RGBA;
        } else {
            cout << "Error: Unsupported channel number.\n";
            stbi_image_free(imageData);
            imageData = nullptr;
        }
        if (imageData) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, imageData);
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
    
    loadedTextures[textureName] = texHandle;
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
    geometryFBO->setBufferSize(windowSize);
    renderFBO->setBufferSize(windowSize);
    bloom1FBO->setBufferSize(windowSize);
    bloom2FBO->setBufferSize(windowSize);
    ssaoFBO->setBufferSize(windowSize / 2);
    ssaoBlurFBO->setBufferSize(windowSize / 2);
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
            camera.moveSpeed_ *= 2.0f;
        } else if (key == GLFW_KEY_DOWN) {
            if (camera.moveSpeed_ > 0.1f) {
                camera.moveSpeed_ /= 2.0f;
            }
        } else if (key == GLFW_KEY_F) {
            flashlightOn = !flashlightOn;
        } else if (key == GLFW_KEY_G) {
            sunlightOn = !sunlightOn;
        } else if (key == GLFW_KEY_H) {
            lampsOn = !lampsOn;
        } else if (key == GLFW_KEY_RIGHT) {
            sunSpeed *= 2.0f;
        } else if (key == GLFW_KEY_LEFT) {
            if (sunSpeed > 0.00001f) {
                sunSpeed /= 2.0f;
            }
        } else if (key == GLFW_KEY_V) {
            config.setVsync(!config.getVsync());
        } else if (key == GLFW_KEY_B) {
            config.setBloom(!config.getBloom());
        } else if (key == GLFW_KEY_N) {
            config.setSSAO(!config.getSSAO());
        } else if (key == GLFW_KEY_T) {
            test = !test;
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
        throw runtime_error("Failed to create GLFW window.");
    }
    glfwMakeContextCurrent(window);
    
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);    // Set function callbacks.
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetScrollCallback(window, scrollCallback);
    
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {    // Load all OpenGL function pointers with GLAD.
        throw runtime_error("Failed to initialize GLAD.");
    }
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDisable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    glfwSetCursorPos(window, windowSize.x / 2.0f, windowSize.y / 2.0f);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    return window;
}

void Simulator::setupTextures() {
    blackTexture = generateTexture(0, 0, 0);
    whiteTexture = generateTexture(255, 255, 255);
    blueTexture = generateTexture(127, 127, 255);
    cubeDiffuseMap = loadTexture("textures/container2.png", true);
    cubeSpecularMap = loadTexture("textures/container2_specular.png", false);
    woodTexture = loadTexture("textures/wood.png", true);
    skyboxCubemap = loadCubemap("textures/skybox/.jpg", true);
    brickDiffuseMap = loadTexture("textures/grid512.bmp", true);
    brickNormalMap = loadTexture("textures/bricks2_normal.jpg", false);
    monitorGridTexture = loadTexture("textures/monitorGrid.png", true);
    
    glGenTextures(1, &ssaoNoiseTexture);
    glBindTexture(GL_TEXTURE_2D, ssaoNoiseTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    vector<glm::vec3> ssaoNoise;
    for (unsigned int i = 0; i < 16; ++i) {
        ssaoNoise.emplace_back(randomFloat(-1.0f, 1.0f), randomFloat(-1.0f, 1.0f), 0.0f);
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 4, 4, 0, GL_RGB, GL_FLOAT, ssaoNoise.data());
}

void Simulator::setupShaders() {
    glGenBuffers(1, &viewProjectionMtxUBO);    // Create a uniform buffer for ViewProjectionMtx.
    glBindBuffer(GL_UNIFORM_BUFFER, viewProjectionMtxUBO);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), nullptr, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, viewProjectionMtxUBO, 0, 2 * sizeof(glm::mat4));    // Link to binding point 0.
    
    geometryShader = make_unique<Shader>("shaders/geometry.v.glsl", "shaders/geometry.f.glsl");
    geometryShader->setUniformBlockBinding("ViewProjectionMtx", 0);
    
    geometryNormalMapShader = make_unique<Shader>("shaders/geometryNormalMap.v.glsl", "shaders/geometryNormalMap.f.glsl");
    geometryNormalMapShader->setUniformBlockBinding("ViewProjectionMtx", 0);
    
    geometrySkinningShader = make_unique<Shader>("shaders/geometrySkinning.v.glsl", "shaders/geometryNormalMap.f.glsl");
    geometrySkinningShader->setUniformBlockBinding("ViewProjectionMtx", 0);
    
    lightingPassShader = make_unique<Shader>("shaders/effects/postProcess.v.glsl", "shaders/effects/lightingPass.f.glsl");
    
    skyboxShader = make_unique<Shader>("shaders/skybox.v.glsl", "shaders/skybox.f.glsl");
    skyboxShader->setUniformBlockBinding("ViewProjectionMtx", 0);
    
    lampShader = make_unique<Shader>("shaders/lamp.v.glsl", "shaders/lamp.f.glsl");
    lampShader->setUniformBlockBinding("ViewProjectionMtx", 0);
    
    shadowMapShader = make_unique<Shader>("shaders/shadowMap.v.glsl", "shaders/shadowMap.f.glsl");
    
    shadowMapSkinningShader = make_unique<Shader>("shaders/shadowMapSkinning.v.glsl", "shaders/shadowMap.f.glsl");
    
    textShader = make_unique<Shader>("shaders/ui/shape.v.glsl", "shaders/ui/text.f.glsl");
    
    shapeShader = make_unique<Shader>("shaders/ui/shape.v.glsl", "shaders/ui/shape.f.glsl");
    
    postProcessShader = make_unique<Shader>("shaders/effects/postProcess.v.glsl", "shaders/effects/postProcess.f.glsl");
    
    bloomShader = make_unique<Shader>("shaders/effects/postProcess.v.glsl", "shaders/effects/bloom.f.glsl");
    
    gaussianBlurShader = make_unique<Shader>("shaders/effects/postProcess.v.glsl", "shaders/effects/gaussianBlur.f.glsl");
    
    ssaoShader = make_unique<Shader>("shaders/effects/postProcess.v.glsl", "shaders/effects/ssao.f.glsl");
    ssaoShader->setUniformBlockBinding("ViewProjectionMtx", 0);
    ssaoShader->use();
    constexpr unsigned int SSAO_NUM_SAMPLES = 32;
    vector<glm::vec3> ssaoSampleKernel;
    ssaoSampleKernel.reserve(SSAO_NUM_SAMPLES);
    for (unsigned int i = 0; i < SSAO_NUM_SAMPLES; ++i) {
        glm::vec3 sample(randomFloat(-1.0f, 1.0f), randomFloat(-1.0f, 1.0f), randomFloat(0.0f, 1.0f));
        float scale = 0.1f + (1.0f - 0.1f) * pow(static_cast<float>(i) / SSAO_NUM_SAMPLES, 2.0f);
        sample = glm::normalize(sample) * randomFloat(0.0f, 1.0f) * scale;
        ssaoSampleKernel.push_back(sample);
    }
    for (unsigned int i = 0; i < SSAO_NUM_SAMPLES; ++i) {
        ssaoShader->setVec3("samples[" + to_string(i) + "]", ssaoSampleKernel[i]);
    }
    
    ssaoBlurShader = make_unique<Shader>("shaders/effects/postProcess.v.glsl", "shaders/effects/ssaoBlur.f.glsl");
}

void Simulator::setupBuffers() {
    geometryFBO = make_unique<Framebuffer>(windowSize);
    geometryFBO->attachTexture(GL_COLOR_ATTACHMENT0, GL_RGBA16F, GL_RGBA, GL_FLOAT, GL_NEAREST, GL_CLAMP_TO_EDGE);    // Position color buffer.
    geometryFBO->attachTexture(GL_COLOR_ATTACHMENT1, GL_RGBA16F, GL_RGBA, GL_FLOAT, GL_NEAREST, GL_CLAMP_TO_EDGE);    // Normal color buffer.
    geometryFBO->attachTexture(GL_COLOR_ATTACHMENT2, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GL_NEAREST, GL_CLAMP_TO_EDGE);    // Albedo and specular color buffer.
    geometryFBO->attachRenderbuffer(GL_DEPTH_ATTACHMENT, GL_DEPTH_COMPONENT);
    geometryFBO->setDrawBuffers({GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2});
    geometryFBO->validate();
    
    renderFBO = make_unique<Framebuffer>(windowSize);
    renderFBO->attachTexture(GL_COLOR_ATTACHMENT0, GL_RGBA16F, GL_RGBA, GL_FLOAT, GL_NEAREST, GL_CLAMP_TO_EDGE);
    renderFBO->attachRenderbuffer(GL_DEPTH_ATTACHMENT, GL_DEPTH_COMPONENT);    // May want GL_DEPTH_STENCIL_ATTACHMENT, GL_DEPTH24_STENCIL8 in future #######################################################################
    renderFBO->validate();
    
    for (unsigned int i = 0; i < NUM_CASCADED_SHADOWS; ++i) {
        cascadedShadowFBO[i] = make_unique<Framebuffer>(glm::ivec2(2048, 2048));
        cascadedShadowFBO[i]->attachTexture(GL_DEPTH_ATTACHMENT, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT, GL_LINEAR, GL_CLAMP_TO_EDGE);
        cascadedShadowFBO[i]->bindTexture(0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
        cascadedShadowFBO[i]->bind();
        glDrawBuffer(GL_NONE);    // Disable color rendering.
        glReadBuffer(GL_NONE);
        cascadedShadowFBO[i]->validate();
    }
    
    bloom1FBO = make_unique<Framebuffer>(windowSize);
    bloom1FBO->attachTexture(GL_COLOR_ATTACHMENT0, GL_RGBA16F, GL_RGBA, GL_FLOAT, GL_NEAREST, GL_CLAMP_TO_EDGE);
    bloom1FBO->validate();
    
    bloom2FBO = make_unique<Framebuffer>(windowSize);
    bloom2FBO->attachTexture(GL_COLOR_ATTACHMENT0, GL_RGBA16F, GL_RGBA, GL_FLOAT, GL_NEAREST, GL_CLAMP_TO_EDGE);
    bloom2FBO->validate();
    
    ssaoFBO = make_unique<Framebuffer>(windowSize / 2);
    ssaoFBO->attachTexture(GL_COLOR_ATTACHMENT0, GL_RED, GL_RED, GL_FLOAT, GL_NEAREST, GL_CLAMP_TO_EDGE);
    ssaoFBO->validate();
    
    ssaoBlurFBO = make_unique<Framebuffer>(windowSize / 2);
    ssaoBlurFBO->attachTexture(GL_COLOR_ATTACHMENT0, GL_RED, GL_RED, GL_FLOAT, GL_LINEAR, GL_CLAMP_TO_EDGE);
    ssaoBlurFBO->validate();
}

void Simulator::setupSimulation() {
    vector<Mesh::Vertex> windowQuadVertices = {
        {{ 1.0f,  1.0f,  0.0f}, { 1.0f, 1.0f}},
        {{-1.0f,  1.0f,  0.0f}, { 0.0f, 1.0f}},
        {{-1.0f, -1.0f,  0.0f}, { 0.0f, 0.0f}},
        {{ 1.0f, -1.0f,  0.0f}, { 1.0f, 0.0f}}
    };
    vector<unsigned int> windowQuadIndices = {
        0, 1, 2, 2, 3, 0
    };
    windowQuad.generateMesh(move(windowQuadVertices), move(windowQuadIndices));
    
    skybox.generateCube(2.0f);
    
    lightCube.generateCube(0.2f);
    cube1.generateCube();
    sphere1.generateSphere();
    
    sceneTest.loadFile("models/boot_camp/boot_camp.obj");
    sceneTestTransform.setScale(glm::vec3(0.025f, 0.025f, 0.025f));
    sceneTestTransform.setPitchYawRoll(glm::vec3(-glm::pi<float>() / 2.0f, 0.0f, 0.0f));
    
    modelTest.loadFile("models/bob_lamp_update/bob_lamp_update.md5mesh");
    //modelTest.loadFile("models/hellknight/hellknight.md5mesh");
    //modelTest.loadFile("models/spaceship/Intergalactic Spaceship_Blender_2.8_Packed textures.dae");
    modelTestTransform.setScale(glm::vec3(0.3f));
    modelTestTransform.setPosition(glm::vec3(0.0f, 0.0f, 2.0f));
    
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

void Simulator::renderScene(const glm::mat4& viewMtx, const glm::mat4& projectionMtx, bool shadowRender) {
    Shader* shader;
    if (shadowRender) {
        shader = shadowMapShader.get();
        shader->use();
        shader->setMat4("lightSpaceMtx", projectionMtx * viewMtx);
    } else {
        glBindBuffer(GL_UNIFORM_BUFFER, viewProjectionMtxUBO);    // Update uniform buffer.
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(viewMtx));
        glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(projectionMtx));
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        
        shader = geometryNormalMapShader.get();
        shader->use();
        shader->setInt("texDiffuse", 0);
        shader->setInt("texSpecular", 1);
        shader->setInt("texNormal", 2);
        //shader->setFloat("material.shininess", 64.0f);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, blueTexture);
    }
    
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
        cube1.drawGeometry(*shader, modelMtx);
    }
    cube1.drawGeometry(*shader, glm::translate(glm::mat4(1.0f), glm::vec3(3.0f, -2.4f, 3.0f)));
    
    cube1.drawGeometry(*shader, glm::scale(glm::translate(glm::mat4(1.0f), camera.position_), glm::vec3(0.4f, 0.4f, 0.4f)));
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, woodTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, woodTexture);
    cube1.drawGeometry(*shader, glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -3.0f, 0.0f)), glm::vec3(15.0f, 0.2f, 15.0f)));
    
    if (!shadowRender) {    // For some reason, lighting does not work properly when geometryNormalMapShader is used with boot_camp.obj, may want to investigate this ###############################################
        shader = geometryShader.get();
        shader->use();
        shader->setInt("texDiffuse", 0);
        shader->setInt("texSpecular", 1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, whiteTexture);
    }
    sceneTest.draw(*shader, sceneTestTransform.getTransform());
    
    if (shadowRender) {
        shader = shadowMapSkinningShader.get();
        shader->use();
        shader->setMat4("lightSpaceMtx", projectionMtx * viewMtx);
    } else {
        shader = geometrySkinningShader.get();
        shader->use();
        shader->setInt("texDiffuse", 0);
        shader->setInt("texSpecular", 1);
        shader->setInt("texNormal", 2);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, blueTexture);
    }
    
    modelTest.draw(*shader, modelTestTransform.getTransform());
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
    
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
        modelTestTransform.rotate(glm::vec3(-0.1f, 0.0f, 0.0f));
    }
    if (glfwGetKey(window, GLFW_KEY_SEMICOLON) == GLFW_PRESS) {
        modelTestTransform.rotate(glm::vec3(0.1f, 0.0f, 0.0f));
    }
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
        modelTestTransform.rotate(glm::vec3(0.0f, 0.1f, 0.0f));
    }
    if (glfwGetKey(window, GLFW_KEY_APOSTROPHE) == GLFW_PRESS) {
        modelTestTransform.rotate(glm::vec3(0.0f, -0.1f, 0.0f));
    }
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
        modelTestTransform.rotate(glm::vec3(0.0f, 0.0f, -0.1f));
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS) {
        modelTestTransform.rotate(glm::vec3(0.0f, 0.0f, 0.1f));
    }
}
