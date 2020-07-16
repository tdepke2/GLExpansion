#include "Configuration.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

bool Configuration::getVsync() const {
    return vsync_;
}

void Configuration::setVsync(bool state) {
    vsync_ = state;
    if (state) {
        glfwSwapInterval(1);    // Update our screen after at least 1 screen refresh.
    } else {
        glfwSwapInterval(0);
    }
}

bool Configuration::getBloom() const {
    return bloom_;
}

void Configuration::setBloom(bool state) {
    bloom_ = state;
}

bool Configuration::getSSAO() const {
    return SSAO_;
}

void Configuration::setSSAO(bool state) {
    SSAO_ = state;
}
