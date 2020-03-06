#include "Simulator.h"
#include "VertexArrayObject.h"
#include <cassert>
#include <utility>

VertexArrayObject::VertexArrayObject() {
    _vaoHandle = 0;
    for (unsigned int i = 0; i < NUM_BUFFERS; ++i) {
        _vboHandles[i] = 0;
    }
}

VertexArrayObject::~VertexArrayObject() {
    glDeleteBuffers(NUM_BUFFERS, _vboHandles);
    glDeleteVertexArrays(1, &_vaoHandle);
}

void VertexArrayObject::generateVertices(vector<VertexAttributes>&& vertices) {
    assert(_vaoHandle == 0);
    _vertices = vertices;
    
    glGenVertexArrays(1, &_vaoHandle);    // Generate handles for VAO and VBO and send data.
    glBindVertexArray(_vaoHandle);
    glGenBuffers(NUM_BUFFERS, _vboHandles);
    glBindBuffer(GL_ARRAY_BUFFER, _vboHandles[0]);
    glBufferData(GL_ARRAY_BUFFER, _vertices.size() * sizeof(VertexAttributes), _vertices.data(), GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(Simulator::ATTRIBUTE_LOCATION_V_POSITION);    // Specify the position and stride for vertices, normals, and tex coords in the array.
    glVertexAttribPointer(Simulator::ATTRIBUTE_LOCATION_V_POSITION, 3, GL_FLOAT, false, sizeof(VertexAttributes), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(Simulator::ATTRIBUTE_LOCATION_V_NORMAL);
    glVertexAttribPointer(Simulator::ATTRIBUTE_LOCATION_V_NORMAL, 3, GL_FLOAT, false, sizeof(VertexAttributes), reinterpret_cast<void*>(sizeof(float) * 3));
    glEnableVertexAttribArray(Simulator::ATTRIBUTE_LOCATION_V_TEX_COORD);
    glVertexAttribPointer(Simulator::ATTRIBUTE_LOCATION_V_TEX_COORD, 2, GL_FLOAT, false, sizeof(VertexAttributes), reinterpret_cast<void*>(sizeof(float) * 6));
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

void VertexArrayObject::generateSphere() {
    
}

void VertexArrayObject::draw() {
    glBindVertexArray(_vaoHandle);
    glDrawArrays(GL_TRIANGLES, 0, _vertices.size());
}
