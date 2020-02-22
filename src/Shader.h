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
    void setBool(const string& name, bool value) const;
    void setInt(const string& name, int value) const;
    void setFloat(const string& name, float value) const;
    void use();
    
    private:
    unsigned int _programHandle;
    
    static unsigned int _compileShader(const string& filename, GLenum shaderType);
};

#endif