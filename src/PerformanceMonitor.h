#ifndef _PERFORMANCE_MONITOR_H
#define _PERFORMANCE_MONITOR_H

#include "Text.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include <string>

using namespace std;

class PerformanceMonitor {
    public:
    static constexpr unsigned int NUM_QUERIES = 3, NUM_SAMPLES = 50;
    static constexpr glm::vec2 BOX_SIZE = glm::vec2(200.0f, 100.0f);
    string name;
    
    PerformanceMonitor(const string& name, shared_ptr<Font> font);
    ~PerformanceMonitor();
    void startGPUTimer();
    void stopGPUTimer();
    void drawBox() const;
    void drawLine() const;
    void drawText() const;
    
    private:
    unsigned int _boxVAO, _boxVBO, _lineVAO, _lineVBO;
    Text _text;
    glm::vec4 _samples[NUM_SAMPLES];
    unsigned int _startQueries[NUM_QUERIES], _stopQueries[NUM_QUERIES];
    unsigned int _queryIndex;
};

#endif
