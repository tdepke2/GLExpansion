#include "PerformanceMonitor.h"
#include "Shader.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <iostream>

float PerformanceMonitor::lastHeightScale_ = BOX_SIZE_.y / INITIAL_SAMPLE_VALUE_, PerformanceMonitor::heightScale_ = BOX_SIZE_.y / INITIAL_SAMPLE_VALUE_, PerformanceMonitor::sampleMax_ = INITIAL_SAMPLE_VALUE_;
stack<PerformanceMonitor*> PerformanceMonitor::monitorNestStack_;

PerformanceMonitor::PerformanceMonitor(const string& name, shared_ptr<Font> font) {
    modelMtx_ = glm::mat4(1.0f);
    name_ = name;
    
    glGenVertexArrays(1, &boxVAO_);
    glBindVertexArray(boxVAO_);
    glGenBuffers(1, &boxVBO_);
    glBindBuffer(GL_ARRAY_BUFFER, boxVBO_);
    glm::vec4 vertices[6] = {
        {0.0f,        0.0f,        0.0f, 0.0f},
        {BOX_SIZE_.x, 0.0f,        1.0f, 0.0f},
        {0.0f,        BOX_SIZE_.y, 0.0f, 1.0f},
        
        {0.0f,        BOX_SIZE_.y, 0.0f, 1.0f},
        {BOX_SIZE_.x, 0.0f,        1.0f, 0.0f},
        {BOX_SIZE_.x, BOX_SIZE_.y, 1.0f, 1.0f}
    };
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(glm::vec4), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, false, sizeof(glm::vec4), 0);
    
    glGenVertexArrays(1, &lineVAO_);
    glBindVertexArray(lineVAO_);
    glGenBuffers(1, &lineVBO_);
    glBindBuffer(GL_ARRAY_BUFFER, lineVBO_);
    for (unsigned int i = 0; i < NUM_SAMPLES_; ++i) {    // Start all samples at INITIAL_SAMPLE_VALUE_ ms to fix issues with auto-scaling.
        samplesScaled_[i] = glm::vec4(static_cast<float>(i) / (NUM_SAMPLES_ - 1) * BOX_SIZE_.x, 1.0f, 0.0f, 0.0f);
    }
    lastSample_ = INITIAL_SAMPLE_VALUE_;
    sampleAverage_ = INITIAL_SAMPLE_VALUE_;
    glBufferData(GL_ARRAY_BUFFER, NUM_SAMPLES_ * sizeof(glm::vec4), samplesScaled_, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, false, sizeof(glm::vec4), 0);
    
    parentMonitor_ = nullptr;
    text_.setFont(font);
    text_.modelMtx_ = glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, BOX_SIZE_.y - 20.0f, 0.0f));
    glGenQueries(NUM_QUERIES_, startQueries_);
    glGenQueries(NUM_QUERIES_, stopQueries_);
    queryIndex_ = 0;
    for (unsigned int i = 1; i < NUM_QUERIES_; ++i) {    // Prepare other queries with garbage values so that there is no error when reading.
        glQueryCounter(startQueries_[i], GL_TIMESTAMP);
        glQueryCounter(stopQueries_[i], GL_TIMESTAMP);
    }
}

PerformanceMonitor::~PerformanceMonitor() {
    glDeleteVertexArrays(1, &boxVAO_);
    glDeleteBuffers(1, &boxVBO_);
    glDeleteVertexArrays(1, &lineVAO_);
    glDeleteBuffers(1, &lineVBO_);
    glDeleteQueries(NUM_QUERIES_, startQueries_);
    glDeleteQueries(NUM_QUERIES_, stopQueries_);
}

float PerformanceMonitor::getLastSample() const {
    return lastSample_;
}

float PerformanceMonitor::getSampleAverage() const {
    return sampleAverage_;
}

void PerformanceMonitor::startGPUTimer() {
    glQueryCounter(startQueries_[queryIndex_], GL_TIMESTAMP);
    monitorNestStack_.push(this);
}

void PerformanceMonitor::stopGPUTimer() {
    glQueryCounter(stopQueries_[queryIndex_], GL_TIMESTAMP);
    monitorNestStack_.pop();
    if (monitorNestStack_.empty()) {
        parentMonitor_ = nullptr;
    } else {
        parentMonitor_ = monitorNestStack_.top();
    }
    
    queryIndex_ = (queryIndex_ + 1) % NUM_QUERIES_;
    /*int queryDone;
    glGetQueryObjectiv(stopQueries_[queryIndex_], GL_QUERY_RESULT_AVAILABLE, &queryDone);
    if (!queryDone) {
        cout << "Warn: PerformanceMonitor query was measured but not completed.\n";
    }*/
    
    unsigned long long startTime, stopTime;    // Compute elapsed time.
    glGetQueryObjectui64v(startQueries_[queryIndex_], GL_QUERY_RESULT, &startTime);
    glGetQueryObjectui64v(stopQueries_[queryIndex_], GL_QUERY_RESULT, &stopTime);
    lastSample_ = (stopTime - startTime) / 1000000.0f;    // Elapsed time in ms.
    sampleMax_ = max(sampleMax_, lastSample_);
    
    if (monitorNestStack_.empty()) {    // Update height scaling if this is the last monitor to stop.
        lastHeightScale_ = heightScale_;
        heightScale_ += (BOX_SIZE_.y / sampleMax_ - heightScale_) / 8.0f;
        sampleMax_ = 0.0f;
    }
}

void PerformanceMonitor::update() {
    float sampleSumScaled = 0.0f;    // Update the line graph.
    float sampleMaxScaled = 0.0f;
    float scalingRatio = heightScale_ / lastHeightScale_;
    for (unsigned int i = 0; i < NUM_SAMPLES_ - 1; ++i) {
        samplesScaled_[i].y = samplesScaled_[i + 1].y * scalingRatio;
        sampleSumScaled += samplesScaled_[i].y;
        sampleMaxScaled = max(sampleMaxScaled, samplesScaled_[i].y);
    }
    samplesScaled_[NUM_SAMPLES_ - 1].y = lastSample_ * heightScale_;
    sampleSumScaled += samplesScaled_[NUM_SAMPLES_ - 1].y;
    sampleAverage_ = sampleSumScaled / heightScale_ / NUM_SAMPLES_;
    sampleMax_ = max(sampleMax_, sampleMaxScaled / heightScale_);
    
    glBindVertexArray(lineVAO_);
    glBindBuffer(GL_ARRAY_BUFFER, lineVBO_);
    glBufferSubData(GL_ARRAY_BUFFER, 0, NUM_SAMPLES_ * sizeof(glm::vec4), samplesScaled_);
    
    string percentOfParentMonitor = "";    // Update text.
    if (parentMonitor_ != nullptr) {
        percentOfParentMonitor = "\n(" + to_string(sampleAverage_ / parentMonitor_->sampleAverage_ * 100.0f) + "% of " + parentMonitor_->name_ + ")";
    }
    text_.setString(name_ + "\nAvg: " + to_string(sampleAverage_) + " ms" + percentOfParentMonitor);
}

void PerformanceMonitor::drawBox(const Shader& shader, const glm::mat4& modelMtx) const {
    shader.setMat4("modelMtx", modelMtx * modelMtx_);
    glBindVertexArray(boxVAO_);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void PerformanceMonitor::drawLine(const Shader& shader, const glm::mat4& modelMtx) const {
    shader.setMat4("modelMtx", modelMtx * modelMtx_);
    glBindVertexArray(lineVAO_);
    glDrawArrays(GL_LINE_STRIP, 0, NUM_SAMPLES_);
}

void PerformanceMonitor::drawText(const Shader& shader, const glm::mat4& modelMtx) const {
    text_.draw(shader, modelMtx * modelMtx_);
}
