#include "PerformanceMonitor.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

PerformanceMonitor::PerformanceMonitor(const string& name, shared_ptr<Font> font) {
    this->name = name;
    _text.setFont(font);
    glGenQueries(NUM_QUERIES, startQueries);
    glGenQueries(NUM_QUERIES, stopQueries);
    queryIndex = 0;
    for (unsigned int i = 1; i < NUM_QUERIES; ++i) {    // Prepare other queries with garbage values so that there is no error when reading.
        glQueryCounter(startQueries[i], GL_TIMESTAMP);
        glQueryCounter(stopQueries[i], GL_TIMESTAMP);
    }
}

PerformanceMonitor::~PerformanceMonitor() {
    glDeleteQueries(NUM_QUERIES, startQueries);
    glDeleteQueries(NUM_QUERIES, stopQueries);
}

void PerformanceMonitor::startGPUTimer() {
    glQueryCounter(startQueries[queryIndex], GL_TIMESTAMP);
}

void PerformanceMonitor::stopGPUTimer() {
    glQueryCounter(stopQueries[queryIndex], GL_TIMESTAMP);
    
    queryIndex = (queryIndex + 1) % NUM_QUERIES;
    /*int queryDone;
    glGetQueryObjectiv(stopQueries[queryIndex], GL_QUERY_RESULT_AVAILABLE, &queryDone);
    if (!queryDone) {
        cout << "Warn: PerformanceMonitor query was measured but not completed.\n";
    }*/
    
    unsigned long long startTime, stopTime;
    glGetQueryObjectui64v(startQueries[queryIndex], GL_QUERY_RESULT, &startTime);
    glGetQueryObjectui64v(stopQueries[queryIndex], GL_QUERY_RESULT, &stopTime);
    _text.setString("Elapsed: " + to_string((stopTime - startTime) / 1000000.0) + " ms");
}

void PerformanceMonitor::draw() const {
    _text.draw();
}
