#include "Shader.h"
#include <fstream>
#include <iostream>
#include <sstream>

Shader::Shader(const string& vertexShaderPath, const string& fragmentShaderPath) {
    unsigned int vertexShader = _compileShader(vertexShaderPath, GL_VERTEX_SHADER);
    unsigned int fragmentShader = _compileShader(fragmentShaderPath, GL_FRAGMENT_SHADER);
    
    _programHandle = glCreateProgram();    // Create the program and link the shaders.
    glAttachShader(_programHandle, vertexShader);
    glAttachShader(_programHandle, fragmentShader);
    glLinkProgram(_programHandle);
    int success;
    glGetProgramiv(_programHandle, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(_programHandle, 512, nullptr, infoLog);
        cout << "Error: Failed to link shader: " << infoLog << endl;
    }
    
    glDeleteShader(vertexShader);    // Clean up the individual shader parts as they are no longer needed.
    glDeleteShader(fragmentShader);
}

unsigned int Shader::getHandle() const {
    return _programHandle;
}

void Shader::setBool(const string& name, bool value) const {
    glUniform1i(glGetUniformLocation(_programHandle, name.c_str()), static_cast<int>(value));
}

void Shader::setInt(const string& name, int value) const {
    glUniform1i(glGetUniformLocation(_programHandle, name.c_str()), value);
}

void Shader::setFloat(const string& name, float value) const {
    glUniform1f(glGetUniformLocation(_programHandle, name.c_str()), value);
}

void Shader::use() {
    glUseProgram(_programHandle);
}

unsigned int Shader::_compileShader(const string& filename, GLenum shaderType) {
    ifstream inputFile(filename);    // Open file and read the source code.
    if (!inputFile) {
        cout << "Error: Unable to open shader file \"" << filename << "\"." << endl;
        return 0;
    }
    ostringstream fileStream;
    fileStream << inputFile.rdbuf();
    string shaderString = fileStream.str();
    const char* shaderCode = shaderString.c_str();
    inputFile.close();
    
    unsigned int shaderHandle = glCreateShader(shaderType);    // Create shader and compile it.
    glShaderSource(shaderHandle, 1, &shaderCode, nullptr);
    glCompileShader(shaderHandle);
    int success;
    glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shaderHandle, 512, nullptr, infoLog);
        cout << "Error: In shader \"" << filename << "\": " << infoLog << endl;
    }
    
    return shaderHandle;
}