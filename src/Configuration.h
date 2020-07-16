#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

class Configuration {
    public:
    bool getVsync() const;
    void setVsync(bool state);
    bool getBloom() const;
    void setBloom(bool state);
    bool getSSAO() const;
    void setSSAO(bool state);
    
    private:
    bool vsync_, bloom_, SSAO_;
};

#endif
