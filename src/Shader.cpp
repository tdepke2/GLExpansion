#include "Shader.h"
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <sstream>

Shader::Shader(const string& vertexShaderPath, const string& fragmentShaderPath) {
    unsigned int vertexShader = compileShader(vertexShaderPath, GL_VERTEX_SHADER);
    unsigned int fragmentShader = compileShader(fragmentShaderPath, GL_FRAGMENT_SHADER);
    
    programHandle_ = glCreateProgram();    // Create the program and link the shaders.
    glAttachShader(programHandle_, vertexShader);
    glAttachShader(programHandle_, fragmentShader);
    glLinkProgram(programHandle_);
    int success;
    glGetProgramiv(programHandle_, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(programHandle_, 512, nullptr, infoLog);
        cout << "Error: Failed to link shader: " << infoLog << endl;
    }
    
    glDetachShader(programHandle_, vertexShader);    // Clean up the individual shader parts as they are no longer needed.
    glDetachShader(programHandle_, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

Shader::Shader(const string& vertexShaderPath, const string& geometryShaderPath, const string& fragmentShaderPath) {
    unsigned int vertexShader = compileShader(vertexShaderPath, GL_VERTEX_SHADER);
    unsigned int geometryShader = compileShader(geometryShaderPath, GL_GEOMETRY_SHADER);
    unsigned int fragmentShader = compileShader(fragmentShaderPath, GL_FRAGMENT_SHADER);
    
    programHandle_ = glCreateProgram();    // Create the program and link the shaders.
    glAttachShader(programHandle_, vertexShader);
    glAttachShader(programHandle_, geometryShader);
    glAttachShader(programHandle_, fragmentShader);
    glLinkProgram(programHandle_);
    int success;
    glGetProgramiv(programHandle_, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(programHandle_, 512, nullptr, infoLog);
        cout << "Error: Failed to link shader: " << infoLog << endl;
    }
    
    glDetachShader(programHandle_, vertexShader);    // Clean up the individual shader parts as they are no longer needed.
    glDetachShader(programHandle_, geometryShader);
    glDetachShader(programHandle_, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(geometryShader);
    glDeleteShader(fragmentShader);
}

Shader::~Shader() {
    glDeleteProgram(programHandle_);
}

unsigned int Shader::getHandle() const {
    return programHandle_;
}

void Shader::setFloat(const string& name, float value) const {
    glUniform1f(getUniformLocation(name), value);
}

void Shader::setBool(const string& name, bool value) const {
    glUniform1i(getUniformLocation(name), static_cast<int>(value));
}

void Shader::setInt(const string& name, int value) const {
    glUniform1i(getUniformLocation(name), value);
}

void Shader::setUnsignedInt(const string& name, unsigned int value) const {
    glUniform1ui(getUniformLocation(name), value);
}

void Shader::setFloatArray(const string& name, unsigned int count, const float* valuePtr) const {
    glUniform1fv(getUniformLocation(name), count, valuePtr);
}

void Shader::setIntArray(const string& name, unsigned int count, const int* valuePtr) const {
    glUniform1iv(getUniformLocation(name), count, valuePtr);
}

void Shader::setUnsignedIntArray(const string& name, unsigned int count, const unsigned int* valuePtr) const {
    glUniform1uiv(getUniformLocation(name), count, valuePtr);
}

void Shader::setVec2(const string& name, const glm::vec2& value) const {
    glUniform2fv(getUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::setVec2(const string& name, float x, float y) const {
    glUniform2f(getUniformLocation(name), x, y);
}

void Shader::setVec3(const string& name, const glm::vec3& value) const {
    glUniform3fv(getUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::setVec3(const string& name, float x, float y, float z) const {
    glUniform3f(getUniformLocation(name), x, y, z);
}

void Shader::setVec4(const string& name, const glm::vec4& value) const {
    glUniform4fv(getUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::setVec4(const string& name, float x, float y, float z, float w) const {
    glUniform4f(getUniformLocation(name), x, y, z, w);
}

void Shader::setVec2Array(const string& name, unsigned int count, const glm::vec2* valuePtr) const {
    glUniform2fv(getUniformLocation(name), count, glm::value_ptr(valuePtr[0]));
}

void Shader::setVec3Array(const string& name, unsigned int count, const glm::vec3* valuePtr) const {
    glUniform3fv(getUniformLocation(name), count, glm::value_ptr(valuePtr[0]));
}

void Shader::setVec4Array(const string& name, unsigned int count, const glm::vec4* valuePtr) const {
    glUniform4fv(getUniformLocation(name), count, glm::value_ptr(valuePtr[0]));
}

void Shader::setMat2(const string& name, const glm::mat2& value) const {
    glUniformMatrix2fv(getUniformLocation(name), 1, false, glm::value_ptr(value));
}

void Shader::setMat3(const string& name, const glm::mat3& value) const {
    glUniformMatrix3fv(getUniformLocation(name), 1, false, glm::value_ptr(value));
}

void Shader::setMat4(const string& name, const glm::mat4& value) const {
    glUniformMatrix4fv(getUniformLocation(name), 1, false, glm::value_ptr(value));
}

void Shader::setMat2Array(const string& name, unsigned int count, const glm::mat2* valuePtr) const {
    glUniformMatrix2fv(getUniformLocation(name), count, false, glm::value_ptr(valuePtr[0]));
}

void Shader::setMat3Array(const string& name, unsigned int count, const glm::mat3* valuePtr) const {
    glUniformMatrix3fv(getUniformLocation(name), count, false, glm::value_ptr(valuePtr[0]));
}

void Shader::setMat4Array(const string& name, unsigned int count, const glm::mat4* valuePtr) const {
    glUniformMatrix4fv(getUniformLocation(name), count, false, glm::value_ptr(valuePtr[0]));
}

void Shader::setUniformBlockBinding(const string& name, unsigned int value) const {
    unsigned int index = glGetUniformBlockIndex(programHandle_, name.c_str());
    if (index == GL_INVALID_INDEX) {
        cout << "Error: Failed to set uniform block binding \"" << name << "\".\n";
    }
    glUniformBlockBinding(programHandle_, index, value);
}

void Shader::use() const {
    glUseProgram(programHandle_);
}

unsigned int Shader::compileShader(const string& filename, GLenum shaderType) {
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

int Shader::getUniformLocation(const string& name) const {
    int location = glGetUniformLocation(programHandle_, name.c_str());
    if (location == -1) {
        cout << "Error: Failed to set uniform \"" << name << "\".\n";
    }
    return location;
}
