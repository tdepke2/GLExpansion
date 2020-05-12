#include "Shader.h"
#include <fstream>
#include <iostream>
#include <stdexcept>
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
    
    glDetachShader(_programHandle, vertexShader);    // Clean up the individual shader parts as they are no longer needed.
    glDetachShader(_programHandle, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

Shader::~Shader() {
    glDeleteProgram(_programHandle);
}

unsigned int Shader::getHandle() const {
    return _programHandle;
}

void Shader::setFloat(const string& name, float value) const {
    glUniform1f(_getUniformLocation(name), value);
}

void Shader::setBool(const string& name, bool value) const {
    glUniform1i(_getUniformLocation(name), static_cast<int>(value));
}

void Shader::setInt(const string& name, int value) const {
    glUniform1i(_getUniformLocation(name), value);
}

void Shader::setUnsignedInt(const string& name, unsigned int value) const {
    glUniform1ui(_getUniformLocation(name), value);
}

void Shader::setFloatArray(const string& name, unsigned int count, const float* valuePtr) const {
    glUniform1fv(_getUniformLocation(name), count, valuePtr);
}

void Shader::setIntArray(const string& name, unsigned int count, const int* valuePtr) const {
    glUniform1iv(_getUniformLocation(name), count, valuePtr);
}

void Shader::setUnsignedIntArray(const string& name, unsigned int count, const unsigned int* valuePtr) const {
    glUniform1uiv(_getUniformLocation(name), count, valuePtr);
}

void Shader::setVec2(const string& name, const glm::vec2& value) const {
    glUniform2fv(_getUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::setVec2(const string& name, float x, float y) const {
    glUniform2f(_getUniformLocation(name), x, y);
}

void Shader::setVec3(const string& name, const glm::vec3& value) const {
    glUniform3fv(_getUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::setVec3(const string& name, float x, float y, float z) const {
    glUniform3f(_getUniformLocation(name), x, y, z);
}

void Shader::setVec4(const string& name, const glm::vec4& value) const {
    glUniform4fv(_getUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::setVec4(const string& name, float x, float y, float z, float w) const {
    glUniform4f(_getUniformLocation(name), x, y, z, w);
}

void Shader::setMat2(const string& name, const glm::mat2& value) const {
    glUniformMatrix2fv(_getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::setMat3(const string& name, const glm::mat3& value) const {
    glUniformMatrix3fv(_getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::setMat4(const string& name, const glm::mat4& value) const {
    glUniformMatrix4fv(_getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::setUniformBlockBinding(const string& name, unsigned int value) const {
    unsigned int index = glGetUniformBlockIndex(_programHandle, name.c_str());
    if (index == GL_INVALID_INDEX) {
        cout << "Error: Failed to set uniform block binding \"" << name << "\".\n";
    }
    glUniformBlockBinding(_programHandle, index, value);
}

void Shader::use() const {
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
        return 0;
    }
    
    return shaderHandle;
}

int Shader::_getUniformLocation(const string& name) const {
    int location = glGetUniformLocation(_programHandle, name.c_str());
    if (location == -1) {
        cout << "Error: Failed to set uniform \"" << name << "\".\n";
    }
    return location;
}
