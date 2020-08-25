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

void processEvent(Renderer& renderer, World& world, const Event& e);

int main(int argc, char** argv) {
    cout << "Initializing setup...\n";
    mt19937 randNumGenerator;
    randNumGenerator.seed(static_cast<unsigned long>(chrono::high_resolution_clock::now().time_since_epoch().count()));
    Renderer renderer(&randNumGenerator);
    
    try {
        World world;
        cout << "Setup complete.\n";
        while (renderer.getState() != Renderer::Exiting) {
            renderer.beginFrame(world);
            renderer.drawShadowMaps(world);
            renderer.geometryPass(world);
            renderer.applySSAO();
            renderer.lightingPass(world);
            renderer.drawLamps(world);
            renderer.drawSkybox();
            renderer.applyBloom();
            renderer.drawPostProcessing();
            renderer.drawGUI();
            renderer.endFrame();
            
            Event e;
            while (renderer.pollEvent(e)) {
                processEvent(renderer, world, e);
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

void processEvent(Renderer& renderer, World& world, const Event& e) {
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
            renderer.camera_.moveSpeed_ *= 2.0f;
        } else if (e.key.code == GLFW_KEY_DOWN) {
            if (renderer.camera_.moveSpeed_ > 0.1f) {
                renderer.camera_.moveSpeed_ /= 2.0f;
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
        if (renderer.getState() == Renderer::Running) {
            renderer.camera_.processMouseMove(static_cast<float>(e.mouseMove.xpos) - renderer.lastMousePos_.x, renderer.lastMousePos_.y - static_cast<float>(e.mouseMove.ypos));
        }
        renderer.lastMousePos_.x = static_cast<float>(e.mouseMove.xpos);
        renderer.lastMousePos_.y = static_cast<float>(e.mouseMove.ypos);
    } else if (e.type == Event::MouseScroll) {
        if (renderer.getState() == Renderer::Running) {
            renderer.camera_.processMouseScroll(static_cast<float>(e.mouseScroll.xoffset), static_cast<float>(e.mouseScroll.yoffset));
        }
    }
}
