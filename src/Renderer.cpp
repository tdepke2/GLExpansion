#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Font.h"
#include "Framebuffer.h"
#include "PerformanceMonitor.h"
#include "Renderer.h"
#include "Shader.h"
#include "World.h"
#include <cassert>
#include <iostream>
#include <stdexcept>
#include <utility>

bool Renderer::instantiated_ = false;
unordered_map<string, unsigned int> Renderer::loadedTextures_;
queue<Event> Renderer::eventQueue_;

GLenum Renderer::glCheckError_(const char* file, int line) {
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

unsigned int Renderer::loadTexture(const string& filename, bool gammaCorrection, bool flip) {
    string textureName = filename + (gammaCorrection ? "-g" : "") + (flip ? "-f" : "");
    auto findResult = loadedTextures_.find(textureName);
    if (findResult != loadedTextures_.end()) {
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
    
    loadedTextures_[textureName] = texHandle;
    return texHandle;
}

unsigned int Renderer::loadCubemap(const string& filename, bool gammaCorrection, bool flip) {
    string textureName = filename + (gammaCorrection ? "-g" : "") + (flip ? "-f" : "");
    auto findResult = loadedTextures_.find(textureName);
    if (findResult != loadedTextures_.end()) {
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
    
    loadedTextures_[textureName] = texHandle;
    return texHandle;
}

unsigned int Renderer::generateTexture(int r, int g, int b) {
    string textureName = "color " + to_string(r) + " " + to_string(g) + " " + to_string(b);
    auto findResult = loadedTextures_.find(textureName);
    if (findResult != loadedTextures_.end()) {
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
    
    loadedTextures_[textureName] = texHandle;
    return texHandle;
}

Renderer::Renderer(mt19937* randNumGenerator) :// always use this style for uniform init #############################################################
    camera_(glm::vec3(0.0f, 1.8f, 2.0f)),
    flashlightOn_(false),
    sunlightOn_(true),
    lampsOn_(false),
    sunT_(0.0f),
    sunSpeed_(1.0f),
    state_(Uninitialized),
    randNumGenerator_(randNumGenerator),
    windowSize_(800, 600),
    boneTransforms_(128) {
    
    assert(!instantiated_);    // Ensure only one instance of Renderer.
    instantiated_ = true;
    lastMousePos_ = glm::vec2(windowSize_.x / 2.0f, windowSize_.y / 2.0f);
    state_ = Running;
    
    setupOpenGL();
    setupTextures();
    setupShaders();
    setupBuffers();
    setupRender();
    
    config_.setVsync(true);
    config_.setBloom(true);
    config_.setSSAO(true);
    
    shared_ptr<Font> arialFont = make_shared<Font>();    // why not combine these two, no reason to reset a font. #####################################################################################
    arialFont->loadFont("fonts/arial.ttf", 15);
    
    performanceMonitors_.emplace(make_pair("FRAME", new PerformanceMonitor("FRAME", arialFont))).first->second->modelMtx_ = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    performanceMonitors_.emplace(make_pair("SSAO", new PerformanceMonitor("SSAO", arialFont))).first->second->modelMtx_ = glm::translate(glm::mat4(1.0f), glm::vec3(220.0f, 0.0f, 0.0f));
    performanceMonitors_.emplace(make_pair("BLOOM", new PerformanceMonitor("BLOOM", arialFont))).first->second->modelMtx_ = glm::translate(glm::mat4(1.0f), glm::vec3(440.0f, 0.0f, 0.0f));
    
    for (size_t i = 0; i < boneTransforms_.size(); ++i) {
        boneTransforms_[i] = glm::mat4(1.0f);
    }
    
    geometrySkinningShader_->use();
    geometrySkinningShader_->setMat4Array("boneTransforms", static_cast<unsigned int>(boneTransforms_.size()), boneTransforms_.data());
    shadowMapSkinningShader_->use();
    shadowMapSkinningShader_->setMat4Array("boneTransforms", static_cast<unsigned int>(boneTransforms_.size()), boneTransforms_.data());
    
    constexpr float SHADOW_BOUND_CORRECTION = 0.8f;    // Split points are exponentially distributed with a linear term. https://developer.download.nvidia.com/SDK/10.5/opengl/src/cascaded_shadow_maps/doc/cascaded_shadow_maps.pdf
    shadowZBounds_[0] = NEAR_PLANE;
    for (unsigned int i = 1; i < NUM_CASCADED_SHADOWS; ++i) {
        shadowZBounds_[i] = SHADOW_BOUND_CORRECTION * NEAR_PLANE * pow(FAR_PLANE / NEAR_PLANE, static_cast<float>(i) / NUM_CASCADED_SHADOWS) + (1.0f - SHADOW_BOUND_CORRECTION) * (NEAR_PLANE + static_cast<float>(i) / NUM_CASCADED_SHADOWS) * (FAR_PLANE - NEAR_PLANE);
    }
    shadowZBounds_[NUM_CASCADED_SHADOWS] = FAR_PLANE;
    
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
    lastTime_ = glfwGetTime();
    lastFrameTime_ = lastTime_;
    frameCounter_ = 0;
}

Renderer::~Renderer() {
    instantiated_ = false;
    
    for (auto& m : performanceMonitors_) {
        delete m.second;
    }
    
    glDeleteBuffers(1, &viewProjectionMtxUBO_);    // Clean up allocated resources.
    geometryShader_.reset();
    geometryNormalMapShader_.reset();
    geometrySkinningShader_.reset();
    lightingPassShader_.reset();
    skyboxShader_.reset();
    lampShader_.reset();
    shadowMapShader_.reset();
    shadowMapSkinningShader_.reset();
    textShader_.reset();
    shapeShader_.reset();
    postProcessShader_.reset();
    bloomShader_.reset();
    gaussianBlurShader_.reset();
    ssaoShader_.reset();
    ssaoBlurShader_.reset();
    
    geometryFBO_.reset();
    renderFBO_.reset();
    for (unsigned int i = 0; i < NUM_CASCADED_SHADOWS; ++i) {
        cascadedShadowFBO_[i].reset();
    }
    bloom1FBO_.reset();
    bloom2FBO_.reset();
    ssaoFBO_.reset();
    ssaoBlurFBO_.reset();
    
    glCheckError();
    glfwDestroyWindow(window_);
    glfwTerminate();
}

Renderer::State Renderer::getState() const {
    return state_;
}

void Renderer::setState(State state) {
    state_ = state;
    if (state == Running) {
        glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    } else if (state == Paused) {
        glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        glfwSetCursorPos(window_, windowSize_.x / 2.0f, windowSize_.y / 2.0f);
    }
}

void Renderer::beginFrame(const World& world) {
    double currentTime = glfwGetTime();
    float deltaTime = static_cast<float>(currentTime - lastTime_);
    lastTime_ = currentTime;
    
    performanceMonitors_.at("FRAME")->startGPUTimer();
    
    ++frameCounter_;
    if (currentTime - lastFrameTime_ >= 1.0) {
        string windowTitle = to_string(frameCounter_) + " FPS (" + to_string(1000.0f / frameCounter_) + " ms/frame)";
        glfwSetWindowTitle(window_, windowTitle.c_str());
        frameCounter_ = 0;
        lastFrameTime_ += 1.0;
        if (currentTime - lastFrameTime_ >= 1.0) {
            lastFrameTime_ = currentTime;
        }
    }
    
    processInput(deltaTime);
    sunT_ += sunSpeed_ * deltaTime;
    
    world.modelTest.animate(0, currentTime, boneTransforms_);
    geometrySkinningShader_->use();
    geometrySkinningShader_->setMat4Array("boneTransforms", static_cast<unsigned int>(boneTransforms_.size()), boneTransforms_.data());
    shadowMapSkinningShader_->use();
    shadowMapSkinningShader_->setMat4Array("boneTransforms", static_cast<unsigned int>(boneTransforms_.size()), boneTransforms_.data());
    
    pointLightPositions_[0] = glm::vec3(0.7f, 0.2f, 2.0f);
    pointLightPositions_[1] = glm::vec3(2.3f, -3.3f, -4.0f);
    pointLightPositions_[2] = glm::vec3(-4.0f, 2.0f, -12.0f);
    pointLightPositions_[3] = glm::vec3(0.0f, 0.0f, -3.0f);
    
    pointLightColors_[0] = glm::vec3(5.0f, 5.0f, 5.0f);
    pointLightColors_[1] = glm::vec3(10.0f, 0.0f, 0.0f);
    pointLightColors_[2] = glm::vec3(0.0f, 0.0f, 15.0f);
    pointLightColors_[3] = glm::vec3(0.0f, 5.0f, 0.0f);
    
    lightStates_[0] = (sunlightOn_ ? 1u : 0u);
    lightStates_[1] = (flashlightOn_ ? 1u : 0u);
    for (int i = 0; i < 4; ++i) {
        lightStates_[i + 2] = (lampsOn_ ? 1u : 0u);
    }
    for (int i = 6; i < NUM_LIGHTS; ++i) {
        lightStates_[i] = 0;
    }
    sunPosition_ = glm::vec3(glm::rotate(glm::mat4(1.0f), sunT_, glm::vec3(1.0f, 1.0f, 1.0f)) * glm::vec4(0.0f, 0.0f, FAR_PLANE, 1.0f));
    if (sunPosition_.x == 0.0f && sunPosition_.z == 0.0f) {    // Fix edge case when directional light aligns with up vector.
        sunPosition_.x = 0.00001f;
    }
}

void Renderer::drawShadowMaps(const World& world) {
    glm::vec2 tanHalfFOV(tan(glm::radians(camera_.fov_ / 2.0f)) * (static_cast<float>(windowSize_.x) / windowSize_.y), tan(glm::radians(camera_.fov_ / 2.0f)));
    glm::mat4 lightViewMtx = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), -sunPosition_, glm::vec3(0.0f, 1.0f, 0.0f));
    viewToLightSpace_ = lightViewMtx * glm::inverse(camera_.getViewMatrix());
    
    for (unsigned int i = 0; i < NUM_CASCADED_SHADOWS; ++i) {    // Calculate an orthographic projection for each of the cascaded shadow volumes.
        float xNear = shadowZBounds_[i] * tanHalfFOV.x;
        float xFar = shadowZBounds_[i + 1] * tanHalfFOV.x;
        float yNear = shadowZBounds_[i] * tanHalfFOV.y;
        float yFar = shadowZBounds_[i + 1] * tanHalfFOV.y;
        
        glm::vec4 frustumCorners[8] = {
            { xNear,  yNear, -shadowZBounds_[i], 1.0f},
            {-xNear,  yNear, -shadowZBounds_[i], 1.0f},
            { xNear, -yNear, -shadowZBounds_[i], 1.0f},
            {-xNear, -yNear, -shadowZBounds_[i], 1.0f},
            
            { xFar,  yFar, -shadowZBounds_[i + 1], 1.0f},
            {-xFar,  yFar, -shadowZBounds_[i + 1], 1.0f},
            { xFar, -yFar, -shadowZBounds_[i + 1], 1.0f},
            {-xFar, -yFar, -shadowZBounds_[i + 1], 1.0f}
        };
        
        glm::vec3 minBound(numeric_limits<float>::max());
        glm::vec3 maxBound(numeric_limits<float>::lowest());
        for (unsigned int j = 0; j < 8; ++j) {
            frustumCorners[j] = viewToLightSpace_ * frustumCorners[j];    // Convert the frustum corner to light space.
            
            minBound.x = min(minBound.x, frustumCorners[j].x);
            minBound.y = min(minBound.y, frustumCorners[j].y);
            minBound.z = min(minBound.z, frustumCorners[j].z);
            
            maxBound.x = max(maxBound.x, frustumCorners[j].x);
            maxBound.y = max(maxBound.y, frustumCorners[j].y);
            maxBound.z = max(maxBound.z, frustumCorners[j].z);
        }
        
        constexpr float NEAR_PLANE_PADDING = FAR_PLANE;    // Extra padding added to near plane to extend the shadow volume behind the camera.
        shadowProjections_[i] = glm::ortho(minBound.x, maxBound.x, minBound.y, maxBound.y, -maxBound.z - NEAR_PLANE_PADDING, -minBound.z);
    }
    
    glViewport(0, 0, cascadedShadowFBO_[0]->getBufferSize().x, cascadedShadowFBO_[0]->getBufferSize().y);
    glCullFace(GL_FRONT);
    for (unsigned int i = 0; i < NUM_CASCADED_SHADOWS; ++i) {    // Render to cascaded shadow maps.
        cascadedShadowFBO_[i]->bind();
        glClear(GL_DEPTH_BUFFER_BIT);
        renderScene(world, lightViewMtx, shadowProjections_[i], true);
    }
    glCullFace(GL_BACK);
}

void Renderer::geometryPass(const World& world) {
    geometryFBO_->bind();    // Render to geometry buffer (geometry pass).
    glViewport(0, 0, geometryFBO_->getBufferSize().x, geometryFBO_->getBufferSize().y);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glm::mat4 viewMtx = camera_.getViewMatrix();
    glm::mat4 projectionMtx = glm::perspective(glm::radians(camera_.fov_), static_cast<float>(windowSize_.x) / windowSize_.y, NEAR_PLANE, FAR_PLANE);
    renderScene(world, viewMtx, projectionMtx, false);
    
    geometryFBO_->bind(GL_READ_FRAMEBUFFER);    // Copy depth buffer over.
    renderFBO_->bind(GL_DRAW_FRAMEBUFFER);
    glBlitFramebuffer(0, 0, geometryFBO_->getBufferSize().x, geometryFBO_->getBufferSize().y, 0, 0, renderFBO_->getBufferSize().x, renderFBO_->getBufferSize().y, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
}

void Renderer::applySSAO() {
    performanceMonitors_.at("SSAO")->startGPUTimer();
    if (config_.getSSAO()) {
        ssaoFBO_->bind();    // Render SSAO texture.
        glViewport(0, 0, ssaoFBO_->getBufferSize().x, ssaoFBO_->getBufferSize().y);
        glClear(GL_COLOR_BUFFER_BIT);
        ssaoShader_->use();
        ssaoShader_->setInt("texPosition", 0);
        ssaoShader_->setInt("texNormal", 1);
        ssaoShader_->setInt("texNoise", 2);
        ssaoShader_->setVec2("noiseScale", glm::vec2(ssaoFBO_->getBufferSize().x / 4.0f, ssaoFBO_->getBufferSize().y / 4.0f));
        glActiveTexture(GL_TEXTURE0);
        geometryFBO_->bindTexture(0);
        glActiveTexture(GL_TEXTURE1);
        geometryFBO_->bindTexture(1);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, ssaoNoiseTexture_);
        windowQuad_.drawGeometry();
        
        ssaoBlurFBO_->bind();    // Blur SSAO texture.
        glViewport(0, 0, ssaoBlurFBO_->getBufferSize().x, ssaoBlurFBO_->getBufferSize().y);
        glClear(GL_COLOR_BUFFER_BIT);
        ssaoBlurShader_->use();
        ssaoBlurShader_->setInt("image", 0);
        glActiveTexture(GL_TEXTURE0);
        ssaoFBO_->bindTexture(0);
        windowQuad_.drawGeometry();
    }
    performanceMonitors_.at("SSAO")->stopGPUTimer();
}

void Renderer::lightingPass() {
    renderFBO_->bind();    // Render lighting (lighting pass).
    glViewport(0, 0, renderFBO_->getBufferSize().x, renderFBO_->getBufferSize().y);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glm::mat4 viewMtx = camera_.getViewMatrix();
    lightingPassShader_->use();
    lightingPassShader_->setInt("texPosition", 0);
    glActiveTexture(GL_TEXTURE0);
    geometryFBO_->bindTexture(0);
    lightingPassShader_->setInt("texNormal", 1);
    glActiveTexture(GL_TEXTURE1);
    geometryFBO_->bindTexture(1);
    lightingPassShader_->setInt("texAlbedoSpec", 2);
    glActiveTexture(GL_TEXTURE2);
    geometryFBO_->bindTexture(2);
    lightingPassShader_->setInt("texSSAO", 3);
    if (config_.getSSAO()) {
        glActiveTexture(GL_TEXTURE3);
        ssaoBlurFBO_->bindTexture(0);
    }
    for (unsigned int i = 0; i < NUM_CASCADED_SHADOWS; ++i) {
        lightingPassShader_->setInt("shadowMap[" + to_string(i) + "]", 4 + i);
        glActiveTexture(GL_TEXTURE4 + i);
        cascadedShadowFBO_[i]->bindTexture(0);
        lightingPassShader_->setMat4("viewToLightSpace[" + to_string(i) + "]", shadowProjections_[i] * viewToLightSpace_);
        lightingPassShader_->setFloat("shadowZEnds[" + to_string(i) + "]", shadowZBounds_[i + 1]);
    }
    lightingPassShader_->setBool("applySSAO", config_.getSSAO());
    lightingPassShader_->setUnsignedIntArray("lightStates", NUM_LIGHTS, lightStates_);
    lightingPassShader_->setUnsignedInt("lights[0].type", 0);
    lightingPassShader_->setVec3("lights[0].directionViewSpace", viewMtx * glm::vec4(-sunPosition_, 0.0f));
    glm::vec3 directionalLightColor(1.0f, 1.0f, 1.0f);
    lightingPassShader_->setVec3("lights[0].ambient", directionalLightColor * 0.05f);
    lightingPassShader_->setVec3("lights[0].diffuse", directionalLightColor * 0.4f);
    lightingPassShader_->setVec3("lights[0].specular", directionalLightColor * 0.5f);
    lightingPassShader_->setUnsignedInt("lights[1].type", 2);
    lightingPassShader_->setVec3("lights[1].positionViewSpace", viewMtx * glm::vec4(camera_.position_, 1.0f));
    lightingPassShader_->setVec3("lights[1].directionViewSpace", viewMtx * glm::vec4(camera_.front_, 0.0f));
    glm::vec3 spotLightColor(1.0f, 1.0f, 1.0f);
    lightingPassShader_->setVec3("lights[1].ambient", spotLightColor * 0.0f);
    lightingPassShader_->setVec3("lights[1].diffuse", spotLightColor);
    lightingPassShader_->setVec3("lights[1].specular", spotLightColor);
    lightingPassShader_->setVec3("lights[1].attenuationVals", glm::vec3(1.0f, 0.09f, 0.032f));
    lightingPassShader_->setVec2("lights[1].cutOff", glm::vec2(glm::cos(glm::radians(12.5f)), glm::cos(glm::radians(17.5f))));
    for (int i = 0; i < 4; ++i) {
        lightingPassShader_->setUnsignedInt("lights[" + to_string(i + 2) + "].type", 1);
        lightingPassShader_->setVec3("lights[" + to_string(i + 2) + "].positionViewSpace", viewMtx * glm::vec4(pointLightPositions_[i], 1.0f));
        lightingPassShader_->setVec3("lights[" + to_string(i + 2) + "].ambient", pointLightColors_[i] * 0.05f);
        lightingPassShader_->setVec3("lights[" + to_string(i + 2) + "].diffuse", pointLightColors_[i] * 0.8f);
        lightingPassShader_->setVec3("lights[" + to_string(i + 2) + "].specular", pointLightColors_[i]);
        lightingPassShader_->setVec3("lights[" + to_string(i + 2) + "].attenuationVals", glm::vec3(1.0f, 0.09f, 0.032f));
    }
    windowQuad_.drawGeometry();
    glEnable(GL_DEPTH_TEST);
}

void Renderer::drawLamps(const World& world) {
    lampShader_->use();    // Draw lamps.
    for (int i = 0; i < 4; ++i) {
        if (lightStates_[i + 2] == 1) {
            lampShader_->setVec3("lightColor", pointLightColors_[i]);
            world.lightCube.drawGeometry(*lampShader_, glm::translate(glm::mat4(1.0f), pointLightPositions_[i]));
        }
    }
    lampShader_->setVec3("lightColor", glm::vec3(1.0f, 1.0f, 0.0f));
    world.lightCube.drawGeometry(*lampShader_, glm::scale(glm::translate(glm::mat4(1.0f), sunPosition_ + camera_.position_), glm::vec3(5.0f)));
}

void Renderer::drawSkybox() {
    skyboxShader_->use();    // Draw the skybox.
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_CULL_FACE);
    skyboxShader_->setInt("skybox", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxCubemap_);
    skybox_.drawGeometry();
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
}

void Renderer::applyBloom() {
    performanceMonitors_.at("BLOOM")->startGPUTimer();
    if (config_.getBloom()) {
        bloom1FBO_->bind();    // Compute bloom texture.
        glViewport(0, 0, bloom1FBO_->getBufferSize().x, bloom1FBO_->getBufferSize().y);
        glClear(GL_COLOR_BUFFER_BIT);
        bloomShader_->use();
        bloomShader_->setInt("image", 0);
        glActiveTexture(GL_TEXTURE0);
        renderFBO_->bindTexture(0);
        windowQuad_.drawGeometry();
        
        gaussianBlurShader_->use();    // Apply Gaussian blur to texture.
        gaussianBlurShader_->setInt("image", 0);
        glActiveTexture(GL_TEXTURE0);
        for (unsigned int i = 0; i < 5; ++i) {
            bloom2FBO_->bind();    // Draw to bloom 2 framebuffer.
            gaussianBlurShader_->setBool("blurHorizontal", true);
            bloom1FBO_->bindTexture(0);
            windowQuad_.drawGeometry();
            
            bloom1FBO_->bind();    // Draw to bloom 1 framebuffer.
            gaussianBlurShader_->setBool("blurHorizontal", false);
            bloom2FBO_->bindTexture(0);
            windowQuad_.drawGeometry();
        }
    }
    performanceMonitors_.at("BLOOM")->stopGPUTimer();
}

void Renderer::drawPostProcessing() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);    // Apply post-processing and render to window.
    glViewport(0, 0, windowSize_.x, windowSize_.y);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    postProcessShader_->use();
    postProcessShader_->setInt("image", 0);
    postProcessShader_->setInt("bloomBlur", 1);
    postProcessShader_->setFloat("exposure", 1.0f);
    postProcessShader_->setBool("applyBloom", config_.getBloom());
    glActiveTexture(GL_TEXTURE0);
    renderFBO_->bindTexture(0);
    if (config_.getBloom()) {
        glActiveTexture(GL_TEXTURE1);
        bloom1FBO_->bindTexture(0);
    }
    windowQuad_.drawGeometry();
    glEnable(GL_DEPTH_TEST);
}

void Renderer::drawGUI() {
    glEnable(GL_BLEND);    // Render GUI.
    glDisable(GL_DEPTH_TEST);
    glm::mat4 windowProjectionMtx = glm::ortho(0.0f, static_cast<float>(windowSize_.x), 0.0f, static_cast<float>(windowSize_.y));
    shapeShader_->use();
    textShader_->setMat4("projectionMtx", windowProjectionMtx);
    shapeShader_->setInt("tex", 0);
    shapeShader_->setVec4("color", glm::vec4(1.0f, 1.0f, 1.0f, 0.7f));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, monitorGridTexture_);
    for (const auto& m : performanceMonitors_) {
        m.second->drawBox(*shapeShader_, glm::mat4(1.0f));
    }
    shapeShader_->setVec4("color", glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
    glBindTexture(GL_TEXTURE_2D, whiteTexture_);
    for (const auto& m : performanceMonitors_) {
        m.second->drawLine(*shapeShader_, glm::mat4(1.0f));
    }
    textShader_->use();
    textShader_->setMat4("projectionMtx", windowProjectionMtx);
    textShader_->setInt("texFont", 0);
    textShader_->setVec3("color", glm::vec3(0.8f, 0.8f, 0.8f));
    for (const auto& m : performanceMonitors_) {
        m.second->drawText(*textShader_, glm::mat4(1.0f));
    }
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

void Renderer::endFrame() {
    performanceMonitors_.at("FRAME")->stopGPUTimer();
    
    for (const auto& m : performanceMonitors_) {    // Monitor update must occur after drawing.
        m.second->update();
    }
    
    glfwSwapBuffers(window_);
    glfwPollEvents();
    glCheckError();
}

bool Renderer::pollEvent(Event& e) {
    if (eventQueue_.empty()) {
        return false;
    }
    
    e = eventQueue_.front();
    eventQueue_.pop();
    return true;
}

void Renderer::resizeBuffers(int width, int height) {
    windowSize_.x = width;
    windowSize_.y = height;
    geometryFBO_->setBufferSize(windowSize_);
    renderFBO_->setBufferSize(windowSize_);
    bloom1FBO_->setBufferSize(windowSize_);
    bloom2FBO_->setBufferSize(windowSize_);
    ssaoFBO_->setBufferSize(windowSize_ / 2);
    ssaoBlurFBO_->setBufferSize(windowSize_ / 2);
}

void Renderer::windowCloseCallback(GLFWwindow* window) {
    Event e;
    e.type = Event::Close;
    eventQueue_.push(e);
}

void Renderer::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    Event e;
    e.type = Event::Resize;
    e.size.width = width;
    e.size.height = height;
    eventQueue_.push(e);
}

void Renderer::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    Event e;
    if (action != GLFW_RELEASE) {
        e.type = Event::KeyPress;
    } else {
        e.type = Event::KeyRelease;
    }
    e.key.code = key;
    e.key.scancode = scancode;
    e.key.mods = mods;
    eventQueue_.push(e);
}

void Renderer::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    Event e;
    if (action != GLFW_RELEASE) {
        e.type = Event::MouseButtonPress;
    } else {
        e.type = Event::MouseButtonRelease;
    }
    e.mouseButton.code = button;
    e.mouseButton.mods = mods;
    eventQueue_.push(e);
}

void Renderer::cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    Event e;
    e.type = Event::MouseMove;
    e.mouseMove.xpos = xpos;
    e.mouseMove.ypos = ypos;
    eventQueue_.push(e);
}

void Renderer::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    Event e;
    e.type = Event::MouseScroll;
    e.mouseScroll.xoffset = xoffset;
    e.mouseScroll.yoffset = yoffset;
    eventQueue_.push(e);
}

void Renderer::setupOpenGL() {
    glfwInit();    // Initialize GLFW and set version.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    window_ = glfwCreateWindow(windowSize_.x, windowSize_.y, "LearnOpenGL", nullptr, nullptr);    // Create the window.
    if (window_ == nullptr) {
        throw runtime_error("Failed to create GLFW window.");
    }
    glfwMakeContextCurrent(window_);
    
    glfwSetWindowCloseCallback(window_, windowCloseCallback);    // Set function callbacks.
    glfwSetFramebufferSizeCallback(window_, framebufferSizeCallback);
    glfwSetKeyCallback(window_, keyCallback);
    glfwSetMouseButtonCallback(window_, mouseButtonCallback);
    glfwSetCursorPosCallback(window_, cursorPosCallback);
    glfwSetScrollCallback(window_, scrollCallback);
    
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
    
    glfwSetCursorPos(window_, windowSize_.x / 2.0f, windowSize_.y / 2.0f);
    glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Renderer::setupTextures() {
    blackTexture_ = generateTexture(0, 0, 0);
    whiteTexture_ = generateTexture(255, 255, 255);
    blueTexture_ = generateTexture(127, 127, 255);
    cubeDiffuseMap_ = loadTexture("textures/container2.png", true);
    cubeSpecularMap_ = loadTexture("textures/container2_specular.png", false);
    woodTexture_ = loadTexture("textures/wood.png", true);
    skyboxCubemap_ = loadCubemap("textures/skybox/.jpg", true);
    brickDiffuseMap_ = loadTexture("textures/grid512.bmp", true);
    brickNormalMap_ = loadTexture("textures/bricks2_normal.jpg", false);
    monitorGridTexture_ = loadTexture("textures/monitorGrid.png", true);
    
    glGenTextures(1, &ssaoNoiseTexture_);
    glBindTexture(GL_TEXTURE_2D, ssaoNoiseTexture_);
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

void Renderer::setupShaders() {
    glGenBuffers(1, &viewProjectionMtxUBO_);    // Create a uniform buffer for ViewProjectionMtx.
    glBindBuffer(GL_UNIFORM_BUFFER, viewProjectionMtxUBO_);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), nullptr, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, viewProjectionMtxUBO_, 0, 2 * sizeof(glm::mat4));    // Link to binding point 0.
    
    geometryShader_ = make_unique<Shader>("shaders/geometry.v.glsl", "shaders/geometry.f.glsl");
    geometryShader_->setUniformBlockBinding("ViewProjectionMtx", 0);
    
    geometryNormalMapShader_ = make_unique<Shader>("shaders/geometryNormalMap.v.glsl", "shaders/geometryNormalMap.f.glsl");
    geometryNormalMapShader_->setUniformBlockBinding("ViewProjectionMtx", 0);
    
    geometrySkinningShader_ = make_unique<Shader>("shaders/geometrySkinning.v.glsl", "shaders/geometryNormalMap.f.glsl");
    geometrySkinningShader_->setUniformBlockBinding("ViewProjectionMtx", 0);
    
    lightingPassShader_ = make_unique<Shader>("shaders/effects/postProcess.v.glsl", "shaders/effects/lightingPass.f.glsl");
    
    skyboxShader_ = make_unique<Shader>("shaders/skybox.v.glsl", "shaders/skybox.f.glsl");
    skyboxShader_->setUniformBlockBinding("ViewProjectionMtx", 0);
    
    lampShader_ = make_unique<Shader>("shaders/lamp.v.glsl", "shaders/lamp.f.glsl");
    lampShader_->setUniformBlockBinding("ViewProjectionMtx", 0);
    
    shadowMapShader_ = make_unique<Shader>("shaders/shadowMap.v.glsl", "shaders/shadowMap.f.glsl");
    
    shadowMapSkinningShader_ = make_unique<Shader>("shaders/shadowMapSkinning.v.glsl", "shaders/shadowMap.f.glsl");
    
    textShader_ = make_unique<Shader>("shaders/ui/shape.v.glsl", "shaders/ui/text.f.glsl");
    
    shapeShader_ = make_unique<Shader>("shaders/ui/shape.v.glsl", "shaders/ui/shape.f.glsl");
    
    postProcessShader_ = make_unique<Shader>("shaders/effects/postProcess.v.glsl", "shaders/effects/postProcess.f.glsl");
    
    bloomShader_ = make_unique<Shader>("shaders/effects/postProcess.v.glsl", "shaders/effects/bloom.f.glsl");
    
    gaussianBlurShader_ = make_unique<Shader>("shaders/effects/postProcess.v.glsl", "shaders/effects/gaussianBlur.f.glsl");
    
    ssaoShader_ = make_unique<Shader>("shaders/effects/postProcess.v.glsl", "shaders/effects/ssao.f.glsl");
    ssaoShader_->setUniformBlockBinding("ViewProjectionMtx", 0);
    ssaoShader_->use();
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
        ssaoShader_->setVec3("samples[" + to_string(i) + "]", ssaoSampleKernel[i]);
    }
    
    ssaoBlurShader_ = make_unique<Shader>("shaders/effects/postProcess.v.glsl", "shaders/effects/ssaoBlur.f.glsl");
}

void Renderer::setupBuffers() {
    geometryFBO_ = make_unique<Framebuffer>(windowSize_);
    geometryFBO_->attachTexture(GL_COLOR_ATTACHMENT0, GL_RGBA16F, GL_RGBA, GL_FLOAT, GL_NEAREST, GL_CLAMP_TO_EDGE);    // Position color buffer.
    geometryFBO_->attachTexture(GL_COLOR_ATTACHMENT1, GL_RGBA16F, GL_RGBA, GL_FLOAT, GL_NEAREST, GL_CLAMP_TO_EDGE);    // Normal color buffer.
    geometryFBO_->attachTexture(GL_COLOR_ATTACHMENT2, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GL_NEAREST, GL_CLAMP_TO_EDGE);    // Albedo and specular color buffer.
    geometryFBO_->attachRenderbuffer(GL_DEPTH_ATTACHMENT, GL_DEPTH_COMPONENT);
    geometryFBO_->setDrawBuffers({GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2});
    geometryFBO_->validate();
    
    renderFBO_ = make_unique<Framebuffer>(windowSize_);
    renderFBO_->attachTexture(GL_COLOR_ATTACHMENT0, GL_RGBA16F, GL_RGBA, GL_FLOAT, GL_NEAREST, GL_CLAMP_TO_EDGE);
    renderFBO_->attachRenderbuffer(GL_DEPTH_ATTACHMENT, GL_DEPTH_COMPONENT);    // May want GL_DEPTH_STENCIL_ATTACHMENT, GL_DEPTH24_STENCIL8 in future #######################################################################
    renderFBO_->validate();
    
    for (unsigned int i = 0; i < NUM_CASCADED_SHADOWS; ++i) {
        cascadedShadowFBO_[i] = make_unique<Framebuffer>(glm::ivec2(2048, 2048));
        cascadedShadowFBO_[i]->attachTexture(GL_DEPTH_ATTACHMENT, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT, GL_LINEAR, GL_CLAMP_TO_EDGE);
        cascadedShadowFBO_[i]->bindTexture(0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
        cascadedShadowFBO_[i]->bind();
        glDrawBuffer(GL_NONE);    // Disable color rendering.
        glReadBuffer(GL_NONE);
        cascadedShadowFBO_[i]->validate();
    }
    
    bloom1FBO_ = make_unique<Framebuffer>(windowSize_);
    bloom1FBO_->attachTexture(GL_COLOR_ATTACHMENT0, GL_RGBA16F, GL_RGBA, GL_FLOAT, GL_NEAREST, GL_CLAMP_TO_EDGE);
    bloom1FBO_->validate();
    
    bloom2FBO_ = make_unique<Framebuffer>(windowSize_);
    bloom2FBO_->attachTexture(GL_COLOR_ATTACHMENT0, GL_RGBA16F, GL_RGBA, GL_FLOAT, GL_NEAREST, GL_CLAMP_TO_EDGE);
    bloom2FBO_->validate();
    
    ssaoFBO_ = make_unique<Framebuffer>(windowSize_ / 2);
    ssaoFBO_->attachTexture(GL_COLOR_ATTACHMENT0, GL_RED, GL_RED, GL_FLOAT, GL_NEAREST, GL_CLAMP_TO_EDGE);
    ssaoFBO_->validate();
    
    ssaoBlurFBO_ = make_unique<Framebuffer>(windowSize_ / 2);
    ssaoBlurFBO_->attachTexture(GL_COLOR_ATTACHMENT0, GL_RED, GL_RED, GL_FLOAT, GL_LINEAR, GL_CLAMP_TO_EDGE);
    ssaoBlurFBO_->validate();
}

void Renderer::setupRender() {
    vector<Mesh::Vertex> windowQuadVertices = {
        {{ 1.0f,  1.0f,  0.0f}, { 1.0f, 1.0f}},
        {{-1.0f,  1.0f,  0.0f}, { 0.0f, 1.0f}},
        {{-1.0f, -1.0f,  0.0f}, { 0.0f, 0.0f}},
        {{ 1.0f, -1.0f,  0.0f}, { 1.0f, 0.0f}}
    };
    vector<unsigned int> windowQuadIndices = {
        0, 1, 2, 2, 3, 0
    };
    windowQuad_.generateMesh(move(windowQuadVertices), move(windowQuadIndices));
    
    skybox_.generateCube(2.0f);
    
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

void Renderer::renderScene(const World& world, const glm::mat4& viewMtx, const glm::mat4& projectionMtx, bool shadowRender) {
    Shader* shader;
    if (shadowRender) {
        shader = shadowMapShader_.get();
        shader->use();
        shader->setMat4("lightSpaceMtx", projectionMtx * viewMtx);
    } else {
        glBindBuffer(GL_UNIFORM_BUFFER, viewProjectionMtxUBO_);    // Update uniform buffer.
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(viewMtx));
        glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(projectionMtx));
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        
        shader = geometryNormalMapShader_.get();
        shader->use();
        shader->setInt("texDiffuse", 0);
        shader->setInt("texSpecular", 1);
        shader->setInt("texNormal", 2);
        //shader->setFloat("material.shininess", 64.0f);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, blueTexture_);
    }
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, cubeDiffuseMap_);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, cubeSpecularMap_);
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
        world.cube1.drawGeometry(*shader, modelMtx);
    }
    world.cube1.drawGeometry(*shader, glm::translate(glm::mat4(1.0f), glm::vec3(3.0f, -2.4f, 3.0f)));
    
    world.cube1.drawGeometry(*shader, glm::scale(glm::translate(glm::mat4(1.0f), camera_.position_), glm::vec3(0.4f, 0.4f, 0.4f)));
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, woodTexture_);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, woodTexture_);
    world.cube1.drawGeometry(*shader, glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -3.0f, 0.0f)), glm::vec3(15.0f, 0.2f, 15.0f)));
    
    if (!shadowRender) {    // For some reason, lighting does not work properly when geometryNormalMapShader is used with boot_camp.obj, may want to investigate this ###############################################
        shader = geometryShader_.get();
        shader->use();
        shader->setInt("texDiffuse", 0);
        shader->setInt("texSpecular", 1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, whiteTexture_);
    }
    world.sceneTest.draw(*shader, world.sceneTestTransform.getTransform());
    
    if (shadowRender) {
        shader = shadowMapSkinningShader_.get();
        shader->use();
        shader->setMat4("lightSpaceMtx", projectionMtx * viewMtx);
    } else {
        shader = geometrySkinningShader_.get();
        shader->use();
        shader->setInt("texDiffuse", 0);
        shader->setInt("texSpecular", 1);
        shader->setInt("texNormal", 2);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, blueTexture_);
    }
    
    world.modelTest.draw(*shader, world.modelTestTransform.getTransform());
}

void Renderer::processInput(float deltaTime) {
    glm::vec3 moveDirection(0.0f, 0.0f, 0.0f);
    if (glfwGetKey(window_, GLFW_KEY_W) == GLFW_PRESS) {
        moveDirection.z -= 1.0f;
    }
    if (glfwGetKey(window_, GLFW_KEY_S) == GLFW_PRESS) {
        moveDirection.z += 1.0f;
    }
    if (glfwGetKey(window_, GLFW_KEY_A) == GLFW_PRESS) {
        moveDirection.x -= 1.0f;
    }
    if (glfwGetKey(window_, GLFW_KEY_D) == GLFW_PRESS) {
        moveDirection.x += 1.0f;
    }
    if (glfwGetKey(window_, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
        moveDirection.y -= 1.0f;
    }
    if (glfwGetKey(window_, GLFW_KEY_SPACE) == GLFW_PRESS) {
        moveDirection.y += 1.0f;
    }
    if (moveDirection != glm::vec3(0.0f, 0.0f, 0.0f)) {
        camera_.processKeyboard(glm::normalize(moveDirection), deltaTime);
    }
}

float Renderer::randomFloat(float min, float max) {
    uniform_real_distribution<float> minMaxRange(min, max);
    return minMaxRange(*randNumGenerator_);
}

int Renderer::randomInt(int min, int max) {
    uniform_int_distribution<int> minMaxRange(min, max);
    return minMaxRange(*randNumGenerator_);
}
