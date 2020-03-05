#ifndef _SHADER_H
#define _SHADER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>

using namespace std;

class Shader {
    public:
    Shader(const string& vertexShaderPath, const string& fragmentShaderPath);
    unsigned int getHandle() const;
    void setFloat(const string& name, float value) const;
    void setInt(const string& name, int value) const;
    void setUnsignedInt(const string& name, unsigned int value) const;
    void setMatrix4Float(const string& name, const float* value) const;
    void use();
    
    private:
    unsigned int _programHandle;
    
    static unsigned int _compileShader(const string& filename, GLenum shaderType);
    int _getUniformLocation(const string& name) const;
};

#endif
