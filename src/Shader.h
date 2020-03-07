#ifndef _SHADER_H
#define _SHADER_H

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
    unsigned int getHandle() const;
    void setFloat(const string& name, float value) const;
    void setBool(const string& name, bool value) const;
    void setInt(const string& name, int value) const;
    void setUnsignedInt(const string& name, unsigned int value) const;
    void setVec2(const string& name, const glm::vec2& value) const;
    void setVec2(const string& name, float x, float y) const;
    void setVec3(const string& name, const glm::vec3& value) const;
    void setVec3(const string& name, float x, float y, float z) const;
    void setVec4(const string& name, const glm::vec4& value) const;
    void setVec4(const string& name, float x, float y, float z, float w) const;
    void setMat2(const string& name, const glm::mat2& value) const;
    void setMat3(const string& name, const glm::mat3& value) const;
    void setMat4(const string& name, const glm::mat4& value) const;
    void use();
    
    private:
    unsigned int _programHandle;
    
    static unsigned int _compileShader(const string& filename, GLenum shaderType);
    int _getUniformLocation(const string& name) const;
};

#endif
