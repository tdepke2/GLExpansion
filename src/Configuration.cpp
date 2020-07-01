#include "Configuration.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

bool Configuration::getVsync() const {
    return _vsync;
}

void Configuration::setVsync(bool state) {
    _vsync = state;
    if (state) {
        glfwSwapInterval(1);    // Update our screen after at least 1 screen refresh.
    } else {
        glfwSwapInterval(0);
    }
}

bool Configuration::getBloom() const {
    return _bloom;
}

void Configuration::setBloom(bool state) {
    _bloom = state;
}

bool Configuration::getSSAO() const {
    return _SSAO;
}

void Configuration::setSSAO(bool state) {
    _SSAO = state;
}
