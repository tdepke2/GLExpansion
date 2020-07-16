#ifndef PERFORMANCE_MONITOR_H_
#define PERFORMANCE_MONITOR_H_

class Shader;

#include "Text.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include <stack>
#include <string>

using namespace std;

class PerformanceMonitor {
    public:
    static constexpr unsigned int NUM_QUERIES_ = 3, NUM_SAMPLES_ = 50;
    static constexpr glm::vec2 BOX_SIZE_ = glm::vec2(200.0f, 100.0f);
    static constexpr float HEIGHT_SCALE_ = 10.0f;
    static stack<PerformanceMonitor*> monitorNestStack_;
    glm::mat4 modelMtx_;
    string name_;
    
    PerformanceMonitor(const string& name, shared_ptr<Font> font);
    ~PerformanceMonitor();
    float getLastSample() const;
    float getSampleAverage() const;
    void startGPUTimer();
    void stopGPUTimer();
    void update();
    void drawBox(const Shader& shader, const glm::mat4& modelMtx = glm::mat4(1.0f)) const;
    void drawLine(const Shader& shader, const glm::mat4& modelMtx = glm::mat4(1.0f)) const;
    void drawText(const Shader& shader, const glm::mat4& modelMtx = glm::mat4(1.0f)) const;
    
    private:
    unsigned int boxVAO_, boxVBO_, lineVAO_, lineVBO_;
    Text text_;
    const PerformanceMonitor* parentMonitor_;
    glm::vec4 samplesScaled_[NUM_SAMPLES_];
    float lastSample_, sampleAverage_;
    unsigned int startQueries_[NUM_QUERIES_], stopQueries_[NUM_QUERIES_];
    unsigned int queryIndex_;
};

#endif
