#ifndef SHADER_H_
#define SHADER_H_

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>

using namespace std;

class Shader {
    public:
    Shader(const string& vertexShaderPath, const string& fragmentShaderPath);
    Shader(const string& vertexShaderPath, const string& geometryShaderPath, const string& fragmentShaderPath);
    ~Shader();
    unsigned int getHandle() const;
    void setFloat(const string& name, float value) const;
    void setBool(const string& name, bool value) const;
    void setInt(const string& name, int value) const;
    void setUnsignedInt(const string& name, unsigned int value) const;
    void setFloatArray(const string& name, unsigned int count, const float* valuePtr) const;
    void setIntArray(const string& name, unsigned int count, const int* valuePtr) const;
    void setUnsignedIntArray(const string& name, unsigned int count, const unsigned int* valuePtr) const;
    void setVec2(const string& name, const glm::vec2& value) const;
    void setVec2(const string& name, float x, float y) const;
    void setVec3(const string& name, const glm::vec3& value) const;
    void setVec3(const string& name, float x, float y, float z) const;
    void setVec4(const string& name, const glm::vec4& value) const;
    void setVec4(const string& name, float x, float y, float z, float w) const;
    void setVec2Array(const string& name, unsigned int count, const glm::vec2* valuePtr) const;
    void setVec3Array(const string& name, unsigned int count, const glm::vec3* valuePtr) const;
    void setVec4Array(const string& name, unsigned int count, const glm::vec4* valuePtr) const;
    void setMat2(const string& name, const glm::mat2& value) const;
    void setMat3(const string& name, const glm::mat3& value) const;
    void setMat4(const string& name, const glm::mat4& value) const;
    void setMat2Array(const string& name, unsigned int count, const glm::mat2* valuePtr) const;
    void setMat3Array(const string& name, unsigned int count, const glm::mat3* valuePtr) const;
    void setMat4Array(const string& name, unsigned int count, const glm::mat4* valuePtr) const;
    void setUniformBlockBinding(const string& name, unsigned int value) const;
    void use() const;
    
    private:
    unsigned int programHandle_;
    
    static unsigned int compileShader(const string& filename, GLenum shaderType);
    int getUniformLocation(const string& name) const;
};

#endif
