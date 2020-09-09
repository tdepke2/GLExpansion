#include "Camera.h"
#include "Event.h"
#include "Renderer.h"
#include "World.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <chrono>
#include <iostream>

using namespace std;

void processInput(Renderer& renderer, Camera& camera, World& world);
void processEvent(Renderer& renderer, Camera& camera, World& world, const Event& e);

int main(int argc, char** argv) {
    cout << "Initializing setup...\n";
    mt19937 randNumGenerator;
    randNumGenerator.seed(static_cast<unsigned long>(chrono::high_resolution_clock::now().time_since_epoch().count()));
    Renderer renderer(&randNumGenerator);
    
    try {
        //Camera camera(glm::vec3(0.0f, 1.8f, 2.0f));
        Camera camera(glm::vec3(-20.0f, 1.8f, -8.0f));
        World world;
        cout << "Setup complete.\n";
        while (renderer.getState() != Renderer::Exiting) {
            renderer.drawWorld(camera, world);
            
            processInput(renderer, camera, world);
            Event e;
            while (renderer.pollEvent(e)) {
                processEvent(renderer, camera, world, e);
            }
            
            world.nextTick();
        }
    } catch (exception& ex) {
        renderer.setState(Renderer::Exiting);
        cout << "\n****************************************************\n";
        cout << "* A fatal error has occurred, terminating program. *\n";
        cout << "****************************************************\n";
        cout << "Error: " << ex.what() << "\n";
        cout << "(Press enter)\n";
        cin.get();
        return -1;
    }
    
    return 0;
}

void processInput(Renderer& renderer, Camera& camera, World& world) {
    glm::vec3 moveDirection(0.0f, 0.0f, 0.0f);
    if (glfwGetKey(renderer.getWindowHandle(), GLFW_KEY_W) == GLFW_PRESS) {
        moveDirection.z -= 1.0f;
    }
    if (glfwGetKey(renderer.getWindowHandle(), GLFW_KEY_S) == GLFW_PRESS) {
        moveDirection.z += 1.0f;
    }
    if (glfwGetKey(renderer.getWindowHandle(), GLFW_KEY_A) == GLFW_PRESS) {
        moveDirection.x -= 1.0f;
    }
    if (glfwGetKey(renderer.getWindowHandle(), GLFW_KEY_D) == GLFW_PRESS) {
        moveDirection.x += 1.0f;
    }
    if (glfwGetKey(renderer.getWindowHandle(), GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
        moveDirection.y -= 1.0f;
    }
    if (glfwGetKey(renderer.getWindowHandle(), GLFW_KEY_SPACE) == GLFW_PRESS) {
        moveDirection.y += 1.0f;
    }
    if (moveDirection != glm::vec3(0.0f, 0.0f, 0.0f)) {
        camera.processKeyboard(glm::normalize(moveDirection));
    }
    
    if (glfwGetKey(renderer.getWindowHandle(), GLFW_KEY_P) == GLFW_PRESS) {
        world.characterTest_.transform_.move(glm::vec3(0.0f, 0.0f, -camera.moveSpeed_));
    }
    if (glfwGetKey(renderer.getWindowHandle(), GLFW_KEY_SEMICOLON) == GLFW_PRESS) {
        world.characterTest_.transform_.move(glm::vec3(0.0f, 0.0f, camera.moveSpeed_));
    }
    if (glfwGetKey(renderer.getWindowHandle(), GLFW_KEY_L) == GLFW_PRESS) {
        world.characterTest_.transform_.move(glm::vec3(-camera.moveSpeed_, 0.0f, 0.0f));
    }
    if (glfwGetKey(renderer.getWindowHandle(), GLFW_KEY_APOSTROPHE) == GLFW_PRESS) {
        world.characterTest_.transform_.move(glm::vec3(camera.moveSpeed_, 0.0f, 0.0f));
    }
    if (glfwGetKey(renderer.getWindowHandle(), GLFW_KEY_COMMA) == GLFW_PRESS) {
        world.characterTest_.transform_.move(glm::vec3(0.0f, -camera.moveSpeed_, 0.0f));
    }
    if (glfwGetKey(renderer.getWindowHandle(), GLFW_KEY_PERIOD) == GLFW_PRESS) {
        world.characterTest_.transform_.move(glm::vec3(0.0f, camera.moveSpeed_, 0.0f));
    }
    
    if (glfwGetKey(renderer.getWindowHandle(), GLFW_KEY_O) == GLFW_PRESS) {
        world.characterTest_.transform_.rotate(glm::vec3(0.0f, -camera.moveSpeed_, 0.0f));
    }
    if (glfwGetKey(renderer.getWindowHandle(), GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS) {
        world.characterTest_.transform_.rotate(glm::vec3(0.0f, camera.moveSpeed_, 0.0f));
    }
}

void processEvent(Renderer& renderer, Camera& camera, World& world, const Event& e) {
    if (e.type == Event::Close) {
        renderer.setState(Renderer::Exiting);
    } else if (e.type == Event::Resize) {
        renderer.resizeBuffers(e.size.width, e.size.height);
    } else if (e.type == Event::KeyPress) {
        if (e.key.code == GLFW_KEY_ESCAPE) {
            if (renderer.getState() == Renderer::Running) {
                renderer.setState(Renderer::Paused);
            } else if (renderer.getState() == Renderer::Paused) {
                renderer.setState(Renderer::Running);
            }
        } else if (e.key.code == GLFW_KEY_UP) {
            camera.moveSpeed_ *= 2.0f;
        } else if (e.key.code == GLFW_KEY_DOWN) {
            if (camera.moveSpeed_ > 0.001f) {
                camera.moveSpeed_ /= 2.0f;
            }
        } else if (e.key.code == GLFW_KEY_F) {
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
            }
        } else if (e.key.code == GLFW_KEY_V) {
            renderer.config_.setVsync(!renderer.config_.getVsync());
        } else if (e.key.code == GLFW_KEY_B) {
            renderer.config_.setBloom(!renderer.config_.getBloom());
        } else if (e.key.code == GLFW_KEY_N) {
            renderer.config_.setSSAO(!renderer.config_.getSSAO());
        }
    } else if (e.type == Event::MouseMove) {
        static glm::vec2 lastMousePos(Renderer::INITIAL_WINDOW_SIZE.x / 2.0f, Renderer::INITIAL_WINDOW_SIZE.y / 2.0f);
        
        if (renderer.getState() == Renderer::Running) {
            camera.processMouseMove(static_cast<float>(e.mouseMove.xpos) - lastMousePos.x, lastMousePos.y - static_cast<float>(e.mouseMove.ypos));
        }
        lastMousePos.x = static_cast<float>(e.mouseMove.xpos);
        lastMousePos.y = static_cast<float>(e.mouseMove.ypos);
    } else if (e.type == Event::MouseScroll) {
        if (renderer.getState() == Renderer::Running) {
            camera.processMouseScroll(static_cast<float>(e.mouseScroll.xoffset), static_cast<float>(e.mouseScroll.yoffset));
        }
    }
}
