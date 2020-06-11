#include "Mesh.h"
#include "Simulator.h"
#include <cassert>
#include <cmath>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <utility>

Mesh::Mesh() {
    _vertexArrayHandle = 0;
    _vertexBufferHandle = 0;
    _elementBufferHandle = 0;
}

Mesh::Mesh(vector<Vertex>&& vertices, vector<unsigned int>&& indices) {
    _vertexArrayHandle = 0;
    _vertexBufferHandle = 0;
    _elementBufferHandle = 0;
    generateMesh(move(vertices), move(indices));
}

Mesh::Mesh(vector<Vertex>&& vertices, vector<unsigned int>&& indices, vector<Texture>&& textures) {
    _vertexArrayHandle = 0;
    _vertexBufferHandle = 0;
    _elementBufferHandle = 0;
    generateMesh(move(vertices), move(indices), move(textures));
}

Mesh::~Mesh() {
    glDeleteVertexArrays(1, &_vertexArrayHandle);
    glDeleteBuffers(1, &_vertexBufferHandle);
    glDeleteBuffers(1, &_elementBufferHandle);
}

Mesh::Mesh(Mesh&& mesh) : _vertexArrayHandle(mesh._vertexArrayHandle), _vertexBufferHandle(mesh._vertexBufferHandle), _elementBufferHandle(mesh._elementBufferHandle) {
    vertices = move(mesh.vertices);
    indices = move(mesh.indices);
    textures = move(mesh.textures);
    mesh._vertexArrayHandle = 0;
    mesh._vertexBufferHandle = 0;
    mesh._elementBufferHandle = 0;
}

Mesh& Mesh::operator=(Mesh&& mesh) {
    vertices = move(mesh.vertices);
    indices = move(mesh.indices);
    textures = move(mesh.textures);
    _vertexArrayHandle = mesh._vertexArrayHandle;
    _vertexBufferHandle = mesh._vertexBufferHandle;
    _elementBufferHandle = mesh._elementBufferHandle;
    mesh._vertexArrayHandle = 0;
    mesh._vertexBufferHandle = 0;
    mesh._elementBufferHandle = 0;
    return *this;
}

void Mesh::generateMesh(vector<Vertex>&& vertices, vector<unsigned int>&& indices) {
    assert(_vertexArrayHandle == 0);
    this->vertices = vertices;
    this->indices = indices;
    
    glGenVertexArrays(1, &_vertexArrayHandle);    // Generate handles for VAO, VBO, and EBO and send data.
    glBindVertexArray(_vertexArrayHandle);
    
    glGenBuffers(1, &_vertexBufferHandle);
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBufferHandle);
    glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(Vertex), this->vertices.data(), GL_STATIC_DRAW);
    
    glGenBuffers(1, &_elementBufferHandle);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _elementBufferHandle);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(unsigned int), this->indices.data(), GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(Simulator::ATTRIBUTE_LOCATION_V_POSITION);    // Specify the position and stride for vertices, normals, and tex coords in the array.
    glVertexAttribPointer(Simulator::ATTRIBUTE_LOCATION_V_POSITION, 3, GL_FLOAT, false, sizeof(Vertex), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(Simulator::ATTRIBUTE_LOCATION_V_NORMAL);
    glVertexAttribPointer(Simulator::ATTRIBUTE_LOCATION_V_NORMAL, 3, GL_FLOAT, false, sizeof(Vertex), reinterpret_cast<void*>(sizeof(float) * 3));
    glEnableVertexAttribArray(Simulator::ATTRIBUTE_LOCATION_V_TEX_COORDS);
    glVertexAttribPointer(Simulator::ATTRIBUTE_LOCATION_V_TEX_COORDS, 2, GL_FLOAT, false, sizeof(Vertex), reinterpret_cast<void*>(sizeof(float) * 6));
}

void Mesh::generateMesh(vector<Vertex>&& vertices, vector<unsigned int>&& indices, vector<Texture>&& textures) {
    generateMesh(move(vertices), move(indices));
    this->textures = textures;
}

void Mesh::generateCube(float sideLength) {
    vector<Vertex> vertices;
    vertices.reserve(24);
    float halfSideLength = sideLength / 2.0f;
    vertices.emplace_back( halfSideLength,  halfSideLength, -halfSideLength,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f);    // Right face.
    vertices.emplace_back( halfSideLength,  halfSideLength,  halfSideLength,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f);
    vertices.emplace_back( halfSideLength, -halfSideLength,  halfSideLength,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f);
    vertices.emplace_back( halfSideLength, -halfSideLength, -halfSideLength,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f);
    
    vertices.emplace_back(-halfSideLength,  halfSideLength,  halfSideLength, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f);    // Left face.
    vertices.emplace_back(-halfSideLength,  halfSideLength, -halfSideLength, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f);
    vertices.emplace_back(-halfSideLength, -halfSideLength, -halfSideLength, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f);
    vertices.emplace_back(-halfSideLength, -halfSideLength,  halfSideLength, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f);
    
    vertices.emplace_back( halfSideLength,  halfSideLength, -halfSideLength,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f);    // Top face.
    vertices.emplace_back(-halfSideLength,  halfSideLength, -halfSideLength,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f);
    vertices.emplace_back(-halfSideLength,  halfSideLength,  halfSideLength,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f);
    vertices.emplace_back( halfSideLength,  halfSideLength,  halfSideLength,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f);
    
    vertices.emplace_back( halfSideLength, -halfSideLength,  halfSideLength,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f);    // Bottom face.
    vertices.emplace_back(-halfSideLength, -halfSideLength,  halfSideLength,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f);
    vertices.emplace_back(-halfSideLength, -halfSideLength, -halfSideLength,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f);
    vertices.emplace_back( halfSideLength, -halfSideLength, -halfSideLength,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f);
    
    vertices.emplace_back( halfSideLength,  halfSideLength,  halfSideLength,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f);    // Front face.
    vertices.emplace_back(-halfSideLength,  halfSideLength,  halfSideLength,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f);
    vertices.emplace_back(-halfSideLength, -halfSideLength,  halfSideLength,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f);
    vertices.emplace_back( halfSideLength, -halfSideLength,  halfSideLength,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f);
    
    vertices.emplace_back(-halfSideLength,  halfSideLength, -halfSideLength,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f);    // Back face.
    vertices.emplace_back( halfSideLength,  halfSideLength, -halfSideLength,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f);
    vertices.emplace_back( halfSideLength, -halfSideLength, -halfSideLength,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f);
    vertices.emplace_back(-halfSideLength, -halfSideLength, -halfSideLength,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f);
    
    vector<unsigned int> indices;
    indices.reserve(36);
    for (unsigned int i = 0; i < 24; i += 4) {
        indices.push_back(i);
        indices.push_back(i + 1);
        indices.push_back(i + 2);
        indices.push_back(i + 2);
        indices.push_back(i + 3);
        indices.push_back(i);
    }
    
    generateMesh(move(vertices), move(indices));
}

void Mesh::generateSphere(float radius, int numSectors, int numStacks) {
    assert(numSectors >= 4 && numStacks >= 2);
    
    vector<Vertex> vertices;    // http://www.songho.ca/opengl/gl_sphere.html#sphere
    vertices.reserve((numSectors + 1) * (numStacks + 1));
    float sectorStep = 2.0f * glm::pi<float>() / numSectors;
    float stackStep = glm::pi<float>() / numStacks;
    for (int i = 0; i <= numStacks; ++i) {
        float stackAngle = glm::pi<float>() / 2.0f - i * stackStep;    // Go from pi/2 to -pi/2.
        float rCosTheta = radius * cos(stackAngle);
        float z = radius * sin(stackAngle);
        
        for (int j = 0; j <= numSectors; ++j) {
            float sectorAngle = j * sectorStep;    // Go from 0 to 2PI.
            
            float x = rCosTheta * cos(sectorAngle);
            float y = rCosTheta * sin(sectorAngle);
            vertices.emplace_back(x, y, z, x / radius, y / radius, z / radius, static_cast<float>(j) / numSectors, 1.0f - static_cast<float>(i) / numStacks);
        }
    }
    
    vector<unsigned int> indices;
    indices.reserve(numSectors * (numStacks - 1) * 6);
    for (int i = 0; i < numStacks; ++i) {
        int index1 = i * (numSectors + 1);    // Index of current stack.
        int index2 = index1 + numSectors + 1;    // Index of next stack.
        
        for (int j = 0; j < numSectors; ++j) {
            if (i != 0) {    // First triangle.
                indices.push_back(index1 + 1);
                indices.push_back(index1);
                indices.push_back(index2);
            }
            
            if (i != numStacks - 1) {    // Second triangle.
                indices.push_back(index2);
                indices.push_back(index2 + 1);
                indices.push_back(index1 + 1);
            }
            ++index1;
            ++index2;
        }
    }
    
    generateMesh(move(vertices), move(indices));
}

void Mesh::applyInstanceBuffer(unsigned int startIndex) const {
    glBindVertexArray(_vertexArrayHandle);
    glEnableVertexAttribArray(startIndex);
    glVertexAttribPointer(startIndex, 4, GL_FLOAT, false, 4 * sizeof(glm::vec4), reinterpret_cast<void*>(0));
    glVertexAttribDivisor(startIndex, 1);
    glEnableVertexAttribArray(startIndex + 1);
    glVertexAttribPointer(startIndex + 1, 4, GL_FLOAT, false, 4 * sizeof(glm::vec4), reinterpret_cast<void*>(1 * sizeof(glm::vec4)));
    glVertexAttribDivisor(startIndex + 1, 1);
    glEnableVertexAttribArray(startIndex + 2);
    glVertexAttribPointer(startIndex + 2, 4, GL_FLOAT, false, 4 * sizeof(glm::vec4), reinterpret_cast<void*>(2 * sizeof(glm::vec4)));
    glVertexAttribDivisor(startIndex + 2, 1);
    glEnableVertexAttribArray(startIndex + 3);
    glVertexAttribPointer(startIndex + 3, 4, GL_FLOAT, false, 4 * sizeof(glm::vec4), reinterpret_cast<void*>(3 * sizeof(glm::vec4)));
    glVertexAttribDivisor(startIndex + 3, 1);
}

void Mesh::draw() const {
    glBindVertexArray(_vertexArrayHandle);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
}

void Mesh::draw(const Shader& shader) const {
    //for (unsigned int i = 0; i < textures.size(); ++i) {
        //glActiveTexture(GL_TEXTURE0 + i);    // ############################################################# Maybe there is no need to pass a shader?
        //glBindTexture(GL_TEXTURE_2D, textures[i].id);
    //}
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures[0].id);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textures[0].id);
    draw();
}

void Mesh::drawInstanced(unsigned int count) const {
    glBindVertexArray(_vertexArrayHandle);
    glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0, count);
}

void Mesh::drawInstanced(const Shader& shader, unsigned int count) const {
    for (unsigned int i = 0; i < textures.size(); ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }
    drawInstanced(count);
}
