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
    static constexpr unsigned int NUM_QUERIES = 3;
    string name;
    
    PerformanceMonitor(const string& name, shared_ptr<Font> font);
    ~PerformanceMonitor();
    void startGPUTimer();
    void stopGPUTimer();
    void draw() const;
    
    private:
    Text _text;
    unsigned int startQueries[NUM_QUERIES], stopQueries[NUM_QUERIES];
    unsigned int queryIndex;
};

#endif
