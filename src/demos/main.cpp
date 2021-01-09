#include "../Camera.h"
#include "../Event.h"
#include "../RenderApp.h"
#include "../Scene.h"
#include "../SceneNode.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

using namespace std;

void processInput(RenderApp& app, Camera* camera);
void processEvent(RenderApp& app, Camera* camera, const Event& e);

int main(int argc, char** argv) {
    cout << "Initializing setup...\n";
    RenderApp app;
    app.init();
    cout << "Setup complete.\n";
    
    Scene* scene = app.createScene();
    
    Camera* camera = scene->createCamera();
    SceneNode* cameraNode = scene->getRootNode()->createChildNode();
    cameraNode->attachObject(camera);
    
    app.startRenderThread();
    while (app.getState() != RenderApp::Exiting) {    // Tick loop.
        app.tempRender();
        
        processInput(app, camera);
        Event e;
        while (app.pollEvent(e)) {
            processEvent(app, camera, e);
        }
    }
    app.close();
    return 0;
}

void processInput(RenderApp& app, Camera* camera) {
    glm::vec3 moveDirection(0.0f, 0.0f, 0.0f);
    if (glfwGetKey(app.getWindowHandle(), GLFW_KEY_W) == GLFW_PRESS) {
        moveDirection.z -= 1.0f;
    }
    if (glfwGetKey(app.getWindowHandle(), GLFW_KEY_S) == GLFW_PRESS) {
        moveDirection.z += 1.0f;
    }
    if (glfwGetKey(app.getWindowHandle(), GLFW_KEY_A) == GLFW_PRESS) {
        moveDirection.x -= 1.0f;
    }
    if (glfwGetKey(app.getWindowHandle(), GLFW_KEY_D) == GLFW_PRESS) {
        moveDirection.x += 1.0f;
    }
    if (glfwGetKey(app.getWindowHandle(), GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
        moveDirection.y -= 1.0f;
    }
    if (glfwGetKey(app.getWindowHandle(), GLFW_KEY_SPACE) == GLFW_PRESS) {
        moveDirection.y += 1.0f;
    }
    if (moveDirection != glm::vec3(0.0f, 0.0f, 0.0f)) {
        camera->processKeyboard(glm::normalize(moveDirection));
    }
    
    /*if (glfwGetKey(app.getWindowHandle(), GLFW_KEY_P) == GLFW_PRESS) {
        world.modelTestTransform_.move(glm::vec3(0.0f, 0.0f, -camera.moveSpeed_));
    }
    if (glfwGetKey(app.getWindowHandle(), GLFW_KEY_SEMICOLON) == GLFW_PRESS) {
        world.modelTestTransform_.move(glm::vec3(0.0f, 0.0f, camera.moveSpeed_));
    }
    if (glfwGetKey(app.getWindowHandle(), GLFW_KEY_L) == GLFW_PRESS) {
        world.modelTestTransform_.move(glm::vec3(-camera.moveSpeed_, 0.0f, 0.0f));
    }
    if (glfwGetKey(app.getWindowHandle(), GLFW_KEY_APOSTROPHE) == GLFW_PRESS) {
        world.modelTestTransform_.move(glm::vec3(camera.moveSpeed_, 0.0f, 0.0f));
    }
    if (glfwGetKey(app.getWindowHandle(), GLFW_KEY_COMMA) == GLFW_PRESS) {
        world.modelTestTransform_.move(glm::vec3(0.0f, -camera.moveSpeed_, 0.0f));
    }
    if (glfwGetKey(app.getWindowHandle(), GLFW_KEY_PERIOD) == GLFW_PRESS) {
        world.modelTestTransform_.move(glm::vec3(0.0f, camera.moveSpeed_, 0.0f));
    }
    if (glfwGetKey(app.getWindowHandle(), GLFW_KEY_O) == GLFW_PRESS) {
        world.modelTestTransform_.rotate(glm::vec3(0.0f, -camera.moveSpeed_, 0.0f));
    }
    if (glfwGetKey(app.getWindowHandle(), GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS) {
        world.modelTestTransform_.rotate(glm::vec3(0.0f, camera.moveSpeed_, 0.0f));
    }*/
}

void processEvent(RenderApp& app, Camera* camera, const Event& e) {
    if (e.type == Event::Close) {
        app.setState(RenderApp::Exiting);
    } else if (e.type == Event::Resize) {
        app.resizeBuffers(e.size.width, e.size.height);
    } else if (e.type == Event::KeyPress) {
        if (e.key.code == GLFW_KEY_ESCAPE) {
            if (app.getState() == RenderApp::Running) {
                app.setState(RenderApp::Paused);
            } else if (app.getState() == RenderApp::Paused) {
                app.setState(RenderApp::Running);
            }
        } else if (e.key.code == GLFW_KEY_UP) {
            camera->moveSpeed_ *= 2.0f;
        } else if (e.key.code == GLFW_KEY_DOWN) {
            if (camera->moveSpeed_ > 0.001f) {
                camera->moveSpeed_ /= 2.0f;
            }
        /*} else if (e.key.code == GLFW_KEY_F) {
            world.flashlightOn_ = !world.flashlightOn_;
        } else if (e.key.code == GLFW_KEY_G) {
            world.sunlightOn_ = !world.sunlightOn_;
        } else if (e.key.code == GLFW_KEY_H) {
            world.lampsOn_ = !world.lampsOn_;
        } else if (e.key.code == GLFW_KEY_RIGHT) {
            world.sunSpeed_ *= 2.0f;
        } else if (e.key.code == GLFW_KEY_LEFT) {
            if (world.sunSpeed_ > 0.00001f) {
                world.sunSpeed_ /= 2.0f;
            }*/
        } else if (e.key.code == GLFW_KEY_V) {
            app.config_.setVsync(!app.config_.getVsync());
        } else if (e.key.code == GLFW_KEY_B) {
            app.config_.setBloom(!app.config_.getBloom());
        } else if (e.key.code == GLFW_KEY_N) {
            app.config_.setSSAO(!app.config_.getSSAO());
        }
    } else if (e.type == Event::MouseMove) {
        static glm::vec2 lastMousePos(RenderApp::INITIAL_WINDOW_SIZE.x / 2.0f, RenderApp::INITIAL_WINDOW_SIZE.y / 2.0f);
        
        if (app.getState() == RenderApp::Running) {
            camera->processMouseMove(static_cast<float>(e.mouseMove.xpos) - lastMousePos.x, lastMousePos.y - static_cast<float>(e.mouseMove.ypos));
        }
        lastMousePos.x = static_cast<float>(e.mouseMove.xpos);
        lastMousePos.y = static_cast<float>(e.mouseMove.ypos);
    } else if (e.type == Event::MouseScroll) {
        if (app.getState() == RenderApp::Running) {
            camera->processMouseScroll(static_cast<float>(e.mouseScroll.xoffset), static_cast<float>(e.mouseScroll.yoffset));
        }
    }
}
