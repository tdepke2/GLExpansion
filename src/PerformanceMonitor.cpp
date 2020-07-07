#include "PerformanceMonitor.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

stack<PerformanceMonitor*> PerformanceMonitor::monitorNestStack;

PerformanceMonitor::PerformanceMonitor(const string& name, shared_ptr<Font> font) {
    modelMtx = glm::mat4(1.0f);
    this->name = name;
    
    glGenVertexArrays(1, &_boxVAO);
    glBindVertexArray(_boxVAO);
    glGenBuffers(1, &_boxVBO);
    glBindBuffer(GL_ARRAY_BUFFER, _boxVBO);
    glm::vec4 vertices[6] = {
        {0.0f,       0.0f,       0.0f, 0.0f},
        {BOX_SIZE.x, 0.0f,       1.0f, 0.0f},
        {0.0f,       BOX_SIZE.y, 0.0f, 1.0f},
        
        {0.0f,       BOX_SIZE.y, 0.0f, 1.0f},
        {BOX_SIZE.x, 0.0f,       1.0f, 0.0f},
        {BOX_SIZE.x, BOX_SIZE.y, 1.0f, 1.0f}
    };
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(glm::vec4), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, false, sizeof(glm::vec4), 0);
    
    glGenVertexArrays(1, &_lineVAO);
    glBindVertexArray(_lineVAO);
    glGenBuffers(1, &_lineVBO);
    glBindBuffer(GL_ARRAY_BUFFER, _lineVBO);
    for (unsigned int i = 0; i < NUM_SAMPLES; ++i) {
        _samplesScaled[i] = glm::vec4(static_cast<float>(i) / (NUM_SAMPLES - 1) * BOX_SIZE.x, 0.0f, 0.0f, 0.0f);
    }
    _lastSample = 0.0f;
    _sampleAverage = 0.0f;
    glBufferData(GL_ARRAY_BUFFER, NUM_SAMPLES * sizeof(glm::vec4), _samplesScaled, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, false, sizeof(glm::vec4), 0);
    
    _parentMonitor = nullptr;
    _text.setFont(font);
    _text.modelMtx = glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, BOX_SIZE.y - 20.0f, 0.0f));
    glGenQueries(NUM_QUERIES, _startQueries);
    glGenQueries(NUM_QUERIES, _stopQueries);
    _queryIndex = 0;
    for (unsigned int i = 1; i < NUM_QUERIES; ++i) {    // Prepare other queries with garbage values so that there is no error when reading.
        glQueryCounter(_startQueries[i], GL_TIMESTAMP);
        glQueryCounter(_stopQueries[i], GL_TIMESTAMP);
    }
}

PerformanceMonitor::~PerformanceMonitor() {
    glDeleteVertexArrays(1, &_boxVAO);
    glDeleteBuffers(1, &_boxVBO);
    glDeleteVertexArrays(1, &_lineVAO);
    glDeleteBuffers(1, &_lineVBO);
    glDeleteQueries(NUM_QUERIES, _startQueries);
    glDeleteQueries(NUM_QUERIES, _stopQueries);
}

float PerformanceMonitor::getLastSample() const {
    return _lastSample;
}

float PerformanceMonitor::getSampleAverage() const {
    return _sampleAverage;
}

void PerformanceMonitor::startGPUTimer() {
    glQueryCounter(_startQueries[_queryIndex], GL_TIMESTAMP);
    monitorNestStack.push(this);
}

void PerformanceMonitor::stopGPUTimer() {
    glQueryCounter(_stopQueries[_queryIndex], GL_TIMESTAMP);
    monitorNestStack.pop();
    if (monitorNestStack.empty()) {
        _parentMonitor = nullptr;
    } else {
        _parentMonitor = monitorNestStack.top();
    }
    
    _queryIndex = (_queryIndex + 1) % NUM_QUERIES;
    /*int queryDone;
    glGetQueryObjectiv(_stopQueries[_queryIndex], GL_QUERY_RESULT_AVAILABLE, &queryDone);
    if (!queryDone) {
        cout << "Warn: PerformanceMonitor query was measured but not completed.\n";
    }*/
    
    unsigned long long startTime, stopTime;    // Compute elapsed time.
    glGetQueryObjectui64v(_startQueries[_queryIndex], GL_QUERY_RESULT, &startTime);
    glGetQueryObjectui64v(_stopQueries[_queryIndex], GL_QUERY_RESULT, &stopTime);
    _lastSample = (stopTime - startTime) / 1000000.0f;    // Elapsed time in ms.
}

void PerformanceMonitor::update() {
    float sampleSumScaled = 0.0f;    // Update the line graph.
    for (unsigned int i = 0; i < NUM_SAMPLES - 1; ++i) {
        _samplesScaled[i].y = _samplesScaled[i + 1].y;
        sampleSumScaled += _samplesScaled[i].y;
    }
    _samplesScaled[NUM_SAMPLES - 1].y = _lastSample * HEIGHT_SCALE;
    sampleSumScaled += _samplesScaled[NUM_SAMPLES - 1].y;
    _sampleAverage = sampleSumScaled / HEIGHT_SCALE / NUM_SAMPLES;
    glBindVertexArray(_lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, _lineVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, NUM_SAMPLES * sizeof(glm::vec4), _samplesScaled);
    
    string percentOfParentMonitor = "";    // Update text.
    if (_parentMonitor != nullptr) {
        percentOfParentMonitor = "\n(" + to_string(_sampleAverage / _parentMonitor->_sampleAverage * 100.0f) + "% of " + _parentMonitor->name + ")";
    }
    _text.setString(name + "\nAvg: " + to_string(_sampleAverage) + " ms" + percentOfParentMonitor);
}

void PerformanceMonitor::drawBox(const Shader* shader, const glm::mat4& modelMtx) const {
    shader->setMat4("modelMtx", modelMtx * this->modelMtx);
    glBindVertexArray(_boxVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void PerformanceMonitor::drawLine(const Shader* shader, const glm::mat4& modelMtx) const {
    shader->setMat4("modelMtx", modelMtx * this->modelMtx);
    glBindVertexArray(_lineVAO);
    glDrawArrays(GL_LINE_STRIP, 0, NUM_SAMPLES);
}

void PerformanceMonitor::drawText(const Shader* shader, const glm::mat4& modelMtx) const {
    _text.draw(shader, modelMtx * this->modelMtx);
}
