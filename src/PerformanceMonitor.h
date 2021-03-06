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
    glm::mat4 modelMtx_;
    string name_;
    
    PerformanceMonitor(const string& name, shared_ptr<Font> font);
    ~PerformanceMonitor();
    float getLastSample() const;
    float getSampleAverage() const;
    void startGPUTimer();
    void stopGPUTimer();
    void update();
    void drawBox(const Shader& shader, const glm::mat4& modelMtx) const;
    void drawLine(const Shader& shader, const glm::mat4& modelMtx) const;
    void drawText(const Shader& shader, const glm::mat4& modelMtx) const;
    
    private:
    static constexpr float INITIAL_SAMPLE_VALUE_ = 1000.0f;
    static float lastHeightScale_, heightScale_, sampleMax_;
    static stack<PerformanceMonitor*> monitorNestStack_;
    unsigned int boxVAO_, boxVBO_, lineVAO_, lineVBO_;
    Text text_;
    const PerformanceMonitor* parentMonitor_;
    glm::vec4 samplesScaled_[NUM_SAMPLES_];
    float lastSample_, sampleAverage_;
    unsigned int startQueries_[NUM_QUERIES_], stopQueries_[NUM_QUERIES_];
    unsigned int queryIndex_;
};

#endif
