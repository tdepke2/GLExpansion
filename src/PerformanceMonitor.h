#ifndef _PERFORMANCE_MONITOR_H
#define _PERFORMANCE_MONITOR_H

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
    static constexpr unsigned int NUM_QUERIES = 3, NUM_SAMPLES = 50;
    static constexpr glm::vec2 BOX_SIZE = glm::vec2(200.0f, 100.0f);
    static constexpr float HEIGHT_SCALE = 10.0f;
    static stack<PerformanceMonitor*> monitorNestStack;
    glm::mat4 modelMtx;
    string name;
    
    PerformanceMonitor(const string& name, shared_ptr<Font> font);
    ~PerformanceMonitor();
    float getLastSample() const;
    float getSampleAverage() const;
    void startGPUTimer();
    void stopGPUTimer();
    void update();
    void drawBox(const Shader* shader, const glm::mat4& modelMtx = glm::mat4(1.0f)) const;
    void drawLine(const Shader* shader, const glm::mat4& modelMtx = glm::mat4(1.0f)) const;
    void drawText(const Shader* shader, const glm::mat4& modelMtx = glm::mat4(1.0f)) const;
    
    private:
    unsigned int _boxVAO, _boxVBO, _lineVAO, _lineVBO;
    Text _text;
    const PerformanceMonitor* _parentMonitor;
    glm::vec4 _samplesScaled[NUM_SAMPLES];
    float _lastSample, _sampleAverage;
    unsigned int _startQueries[NUM_QUERIES], _stopQueries[NUM_QUERIES];
    unsigned int _queryIndex;
};

#endif
