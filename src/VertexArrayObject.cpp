#include "Simulator.h"
#include "VertexArrayObject.h"
#include <cassert>
#include <cmath>
#include <utility>
#define PI acos(-1.0f)

VertexArrayObject::VertexArrayObject() {
    _vertexArrayHandle = 0;
    _vertexBufferHandle = 0;
    _elementBufferHandle = 0;
}

VertexArrayObject::~VertexArrayObject() {
    glDeleteBuffers(1, &_vertexBufferHandle);
    glDeleteBuffers(1, &_elementBufferHandle);
    glDeleteVertexArrays(1, &_vertexArrayHandle);
}

void VertexArrayObject::generateVertices(vector<VertexAttributes>&& vertices) {
    assert(_vertexArrayHandle == 0);
    _vertices = vertices;
    
    glGenVertexArrays(1, &_vertexArrayHandle);    // Generate handles for VAO and VBO and send data.
    glBindVertexArray(_vertexArrayHandle);
    glGenBuffers(1, &_vertexBufferHandle);
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBufferHandle);
    glBufferData(GL_ARRAY_BUFFER, _vertices.size() * sizeof(VertexAttributes), _vertices.data(), GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(Simulator::ATTRIBUTE_LOCATION_V_POSITION);    // Specify the position and stride for vertices, normals, and tex coords in the array.
    glVertexAttribPointer(Simulator::ATTRIBUTE_LOCATION_V_POSITION, 3, GL_FLOAT, false, sizeof(VertexAttributes), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(Simulator::ATTRIBUTE_LOCATION_V_NORMAL);
    glVertexAttribPointer(Simulator::ATTRIBUTE_LOCATION_V_NORMAL, 3, GL_FLOAT, false, sizeof(VertexAttributes), reinterpret_cast<void*>(sizeof(float) * 3));
    glEnableVertexAttribArray(Simulator::ATTRIBUTE_LOCATION_V_TEX_COORDS);
    glVertexAttribPointer(Simulator::ATTRIBUTE_LOCATION_V_TEX_COORDS, 2, GL_FLOAT, false, sizeof(VertexAttributes), reinterpret_cast<void*>(sizeof(float) * 6));
}

void VertexArrayObject::generateVertices(vector<VertexAttributes>&& vertices, vector<unsigned int>&& indices) {
    generateVertices(move(vertices));
    _indices = indices;
    
    glGenBuffers(1, &_elementBufferHandle);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _elementBufferHandle);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indices.size() * sizeof(unsigned int), _indices.data(), GL_STATIC_DRAW);
}

void VertexArrayObject::generateCube(float sideLength) {
    vector<VertexAttributes> vertices;
    vertices.reserve(36);
    float halfSideLength = sideLength / 2.0f;
    vertices.emplace_back( halfSideLength,  halfSideLength, -halfSideLength,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f);    // Right face.
    vertices.emplace_back( halfSideLength,  halfSideLength,  halfSideLength,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f);
    vertices.emplace_back( halfSideLength, -halfSideLength,  halfSideLength,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f);
    vertices.emplace_back( halfSideLength, -halfSideLength,  halfSideLength,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f);
    vertices.emplace_back( halfSideLength, -halfSideLength, -halfSideLength,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f);
    vertices.emplace_back( halfSideLength,  halfSideLength, -halfSideLength,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f);
    
    vertices.emplace_back(-halfSideLength,  halfSideLength,  halfSideLength, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f);    // Left face.
    vertices.emplace_back(-halfSideLength,  halfSideLength, -halfSideLength, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f);
    vertices.emplace_back(-halfSideLength, -halfSideLength, -halfSideLength, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f);
    vertices.emplace_back(-halfSideLength, -halfSideLength, -halfSideLength, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f);
    vertices.emplace_back(-halfSideLength, -halfSideLength,  halfSideLength, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f);
    vertices.emplace_back(-halfSideLength,  halfSideLength,  halfSideLength, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f);
    
    vertices.emplace_back( halfSideLength,  halfSideLength, -halfSideLength,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f);    // Top face.
    vertices.emplace_back(-halfSideLength,  halfSideLength, -halfSideLength,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f);
    vertices.emplace_back(-halfSideLength,  halfSideLength,  halfSideLength,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f);
    vertices.emplace_back(-halfSideLength,  halfSideLength,  halfSideLength,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f);
    vertices.emplace_back( halfSideLength,  halfSideLength,  halfSideLength,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f);
    vertices.emplace_back( halfSideLength,  halfSideLength, -halfSideLength,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f);
    
    vertices.emplace_back( halfSideLength, -halfSideLength,  halfSideLength,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f);    // Bottom face.
    vertices.emplace_back(-halfSideLength, -halfSideLength,  halfSideLength,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f);
    vertices.emplace_back(-halfSideLength, -halfSideLength, -halfSideLength,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f);
    vertices.emplace_back(-halfSideLength, -halfSideLength, -halfSideLength,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f);
    vertices.emplace_back( halfSideLength, -halfSideLength, -halfSideLength,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f);
    vertices.emplace_back( halfSideLength, -halfSideLength,  halfSideLength,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f);
    
    vertices.emplace_back( halfSideLength,  halfSideLength,  halfSideLength,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f);    // Front face.
    vertices.emplace_back(-halfSideLength,  halfSideLength,  halfSideLength,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f);
    vertices.emplace_back(-halfSideLength, -halfSideLength,  halfSideLength,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f);
    vertices.emplace_back(-halfSideLength, -halfSideLength,  halfSideLength,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f);
    vertices.emplace_back( halfSideLength, -halfSideLength,  halfSideLength,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f);
    vertices.emplace_back( halfSideLength,  halfSideLength,  halfSideLength,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f);
    
    vertices.emplace_back(-halfSideLength,  halfSideLength, -halfSideLength,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f);    // Back face.
    vertices.emplace_back( halfSideLength,  halfSideLength, -halfSideLength,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f);
    vertices.emplace_back( halfSideLength, -halfSideLength, -halfSideLength,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f);
    vertices.emplace_back( halfSideLength, -halfSideLength, -halfSideLength,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f);
    vertices.emplace_back(-halfSideLength, -halfSideLength, -halfSideLength,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f);
    vertices.emplace_back(-halfSideLength,  halfSideLength, -halfSideLength,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f);
    generateVertices(move(vertices));
}

void VertexArrayObject::generateSphere(float radius, int numSectors, int numStacks) {
    assert(numSectors >= 4 && numStacks >= 2);
    
    vector<VertexAttributes> vertices;    // http://www.songho.ca/opengl/gl_sphere.html#sphere
    vertices.reserve((numSectors + 1) * (numStacks + 1));
    float sectorStep = 2.0f * PI / numSectors;
    float stackStep = PI / numStacks;
    for (int i = 0; i <= numStacks; ++i) {
        float stackAngle = PI / 2.0f - i * stackStep;    // Go from PI/2 to -PI/2.
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
    
    generateVertices(move(vertices), move(indices));
}

void VertexArrayObject::draw() {
    glBindVertexArray(_vertexArrayHandle);
    if (_indices.empty()) {
        glDrawArrays(GL_TRIANGLES, 0, _vertices.size());
    } else {
        glDrawElements(GL_TRIANGLES, _indices.size(), GL_UNSIGNED_INT, 0);
    }
}
