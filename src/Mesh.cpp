#include "Mesh.h"
#include "Shader.h"
#include "Simulator.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cassert>
#include <cmath>
#include <iostream>
#include <utility>

void Mesh::VertexBone::addBone(uint8_t id, float w) {
    if (weight.x == 0.0f) {
        bone |= static_cast<uint32_t>(id);
        weight.x = w;
    } else if (weight.y == 0.0f) {
        bone |= static_cast<uint32_t>(id) << 8;
        weight.y = w;
    } else if (weight.z == 0.0f) {
        bone |= static_cast<uint32_t>(id) << 16;
        weight.z = w;
    } else if (weight.w == 0.0f) {
        bone |= static_cast<uint32_t>(id) << 24;
        weight.w = w;
    } else {
        cout << "Warn: Vertex with more than four bone attachments not allowed.\n";
    }
}

Mesh::Mesh() {
    vertexArrayHandle_ = 0;
    vertexBufferHandle_ = 0;
    elementBufferHandle_ = 0;
}

Mesh::Mesh(vector<Vertex>&& vertices, vector<unsigned int>&& indices) {
    vertexArrayHandle_ = 0;
    vertexBufferHandle_ = 0;
    elementBufferHandle_ = 0;
    generateMesh(move(vertices), move(indices));
}

Mesh::Mesh(vector<Vertex>&& vertices, vector<unsigned int>&& indices, vector<Texture>&& textures) {
    vertexArrayHandle_ = 0;
    vertexBufferHandle_ = 0;
    elementBufferHandle_ = 0;
    generateMesh(move(vertices), move(indices), move(textures));
}

Mesh::Mesh(vector<VertexBone>&& vertices, vector<unsigned int>&& indices) {
    vertexArrayHandle_ = 0;
    vertexBufferHandle_ = 0;
    elementBufferHandle_ = 0;
    generateMesh(move(vertices), move(indices));
}

Mesh::Mesh(vector<VertexBone>&& vertices, vector<unsigned int>&& indices, vector<Texture>&& textures) {
    vertexArrayHandle_ = 0;
    vertexBufferHandle_ = 0;
    elementBufferHandle_ = 0;
    generateMesh(move(vertices), move(indices), move(textures));
}

Mesh::~Mesh() {
    glDeleteVertexArrays(1, &vertexArrayHandle_);
    glDeleteBuffers(1, &vertexBufferHandle_);
    glDeleteBuffers(1, &elementBufferHandle_);
}

Mesh::Mesh(Mesh&& mesh) : vertexArrayHandle_(mesh.vertexArrayHandle_), vertexBufferHandle_(mesh.vertexBufferHandle_), elementBufferHandle_(mesh.elementBufferHandle_) {
    vertexPositions_ = move(mesh.vertexPositions_);
    indices_ = move(mesh.indices_);
    textures_ = move(mesh.textures_);
    mesh.vertexArrayHandle_ = 0;
    mesh.vertexBufferHandle_ = 0;
    mesh.elementBufferHandle_ = 0;
}

Mesh& Mesh::operator=(Mesh&& mesh) {
    vertexPositions_ = move(mesh.vertexPositions_);
    indices_ = move(mesh.indices_);
    textures_ = move(mesh.textures_);
    vertexArrayHandle_ = mesh.vertexArrayHandle_;
    vertexBufferHandle_ = mesh.vertexBufferHandle_;
    elementBufferHandle_ = mesh.elementBufferHandle_;
    mesh.vertexArrayHandle_ = 0;
    mesh.vertexBufferHandle_ = 0;
    mesh.elementBufferHandle_ = 0;
    return *this;
}

void Mesh::generateMesh(vector<Vertex>&& vertices, vector<unsigned int>&& indices) {
    vertexPositions_.reserve(vertices.size());
    for (const Vertex& v : vertices) {
        vertexPositions_.emplace_back(v.pos);
    }
    indices_ = indices;
    
    generateBuffers();    // Generate handles for VAO, VBO, and EBO and send data.
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_.size() * sizeof(unsigned int), indices_.data(), GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(Simulator::ATTRIBUTE_LOCATION_V_POSITION);    // Specify the position and stride for vertices, normals, and tex coords in the array.
    glVertexAttribPointer(Simulator::ATTRIBUTE_LOCATION_V_POSITION, 3, GL_FLOAT, false, sizeof(Vertex), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(Simulator::ATTRIBUTE_LOCATION_V_NORMAL);
    glVertexAttribPointer(Simulator::ATTRIBUTE_LOCATION_V_NORMAL, 3, GL_FLOAT, false, sizeof(Vertex), reinterpret_cast<void*>(sizeof(float) * 3));
    glEnableVertexAttribArray(Simulator::ATTRIBUTE_LOCATION_V_TEX_COORDS);
    glVertexAttribPointer(Simulator::ATTRIBUTE_LOCATION_V_TEX_COORDS, 2, GL_FLOAT, false, sizeof(Vertex), reinterpret_cast<void*>(sizeof(float) * 6));
    glEnableVertexAttribArray(Simulator::ATTRIBUTE_LOCATION_V_TANGENT);
    glVertexAttribPointer(Simulator::ATTRIBUTE_LOCATION_V_TANGENT, 3, GL_FLOAT, false, sizeof(Vertex), reinterpret_cast<void*>(sizeof(float) * 8));
    glEnableVertexAttribArray(Simulator::ATTRIBUTE_LOCATION_V_BITANGENT);
    glVertexAttribPointer(Simulator::ATTRIBUTE_LOCATION_V_BITANGENT, 3, GL_FLOAT, false, sizeof(Vertex), reinterpret_cast<void*>(sizeof(float) * 11));
}

void Mesh::generateMesh(vector<Vertex>&& vertices, vector<unsigned int>&& indices, vector<Texture>&& textures) {
    generateMesh(move(vertices), move(indices));
    textures_ = textures;
}

void Mesh::generateMesh(vector<VertexBone>&& vertices, vector<unsigned int>&& indices) {
    vertexPositions_.reserve(vertices.size());
    for (const VertexBone& v : vertices) {
        vertexPositions_.emplace_back(v.pos);
    }
    indices_ = indices;
    
    generateBuffers();    // Generate handles for VAO, VBO, and EBO and send data.
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VertexBone), vertices.data(), GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_.size() * sizeof(unsigned int), indices_.data(), GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(Simulator::ATTRIBUTE_LOCATION_V_POSITION);    // Specify the position and stride for vertices, normals, and tex coords in the array.
    glVertexAttribPointer(Simulator::ATTRIBUTE_LOCATION_V_POSITION, 3, GL_FLOAT, false, sizeof(VertexBone), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(Simulator::ATTRIBUTE_LOCATION_V_NORMAL);
    glVertexAttribPointer(Simulator::ATTRIBUTE_LOCATION_V_NORMAL, 3, GL_FLOAT, false, sizeof(VertexBone), reinterpret_cast<void*>(sizeof(float) * 3));
    glEnableVertexAttribArray(Simulator::ATTRIBUTE_LOCATION_V_TEX_COORDS);
    glVertexAttribPointer(Simulator::ATTRIBUTE_LOCATION_V_TEX_COORDS, 2, GL_FLOAT, false, sizeof(VertexBone), reinterpret_cast<void*>(sizeof(float) * 6));
    glEnableVertexAttribArray(Simulator::ATTRIBUTE_LOCATION_V_TANGENT);
    glVertexAttribPointer(Simulator::ATTRIBUTE_LOCATION_V_TANGENT, 3, GL_FLOAT, false, sizeof(VertexBone), reinterpret_cast<void*>(sizeof(float) * 8));
    glEnableVertexAttribArray(Simulator::ATTRIBUTE_LOCATION_V_BITANGENT);
    glVertexAttribPointer(Simulator::ATTRIBUTE_LOCATION_V_BITANGENT, 3, GL_FLOAT, false, sizeof(VertexBone), reinterpret_cast<void*>(sizeof(float) * 11));
    glEnableVertexAttribArray(Simulator::ATTRIBUTE_LOCATION_V_BONE);
    glVertexAttribIPointer(Simulator::ATTRIBUTE_LOCATION_V_BONE, 1, GL_UNSIGNED_INT, sizeof(VertexBone), reinterpret_cast<void*>(sizeof(float) * 14));
    glEnableVertexAttribArray(Simulator::ATTRIBUTE_LOCATION_V_WEIGHT);
    glVertexAttribPointer(Simulator::ATTRIBUTE_LOCATION_V_WEIGHT, 4, GL_FLOAT, false, sizeof(VertexBone), reinterpret_cast<void*>(sizeof(float) * 14 + sizeof(unsigned int)));
}

void Mesh::generateMesh(vector<VertexBone>&& vertices, vector<unsigned int>&& indices, vector<Texture>&& textures) {
    generateMesh(move(vertices), move(indices));
    textures_ = textures;
}

void Mesh::generateCube(float sideLength) {
    vector<Vertex> vertices;
    vertices.reserve(24);
    float halfSideLength = sideLength / 2.0f;
    vertices.emplace_back(glm::vec3( halfSideLength,  halfSideLength, -halfSideLength), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec2( 1.0f,  1.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3( 0.0f,  1.0f,  0.0f));    // Right face.
    vertices.emplace_back(glm::vec3( halfSideLength,  halfSideLength,  halfSideLength), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec2( 0.0f,  1.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3( 0.0f,  1.0f,  0.0f));
    vertices.emplace_back(glm::vec3( halfSideLength, -halfSideLength,  halfSideLength), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec2( 0.0f,  0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3( 0.0f,  1.0f,  0.0f));
    vertices.emplace_back(glm::vec3( halfSideLength, -halfSideLength, -halfSideLength), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec2( 1.0f,  0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3( 0.0f,  1.0f,  0.0f));
    
    vertices.emplace_back(glm::vec3(-halfSideLength,  halfSideLength,  halfSideLength), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec2( 1.0f,  1.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3( 0.0f,  1.0f,  0.0f));    // Left face.
    vertices.emplace_back(glm::vec3(-halfSideLength,  halfSideLength, -halfSideLength), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec2( 0.0f,  1.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3( 0.0f,  1.0f,  0.0f));
    vertices.emplace_back(glm::vec3(-halfSideLength, -halfSideLength, -halfSideLength), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec2( 0.0f,  0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3( 0.0f,  1.0f,  0.0f));
    vertices.emplace_back(glm::vec3(-halfSideLength, -halfSideLength,  halfSideLength), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec2( 1.0f,  0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3( 0.0f,  1.0f,  0.0f));
    
    vertices.emplace_back(glm::vec3( halfSideLength,  halfSideLength, -halfSideLength), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec2( 1.0f,  1.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3( 0.0f,  0.0f, -1.0f));    // Top face.
    vertices.emplace_back(glm::vec3(-halfSideLength,  halfSideLength, -halfSideLength), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec2( 0.0f,  1.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3( 0.0f,  0.0f, -1.0f));
    vertices.emplace_back(glm::vec3(-halfSideLength,  halfSideLength,  halfSideLength), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec2( 0.0f,  0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3( 0.0f,  0.0f, -1.0f));
    vertices.emplace_back(glm::vec3( halfSideLength,  halfSideLength,  halfSideLength), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec2( 1.0f,  0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3( 0.0f,  0.0f, -1.0f));
    
    vertices.emplace_back(glm::vec3( halfSideLength, -halfSideLength,  halfSideLength), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec2( 1.0f,  1.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3( 0.0f,  0.0f,  1.0f));    // Bottom face.
    vertices.emplace_back(glm::vec3(-halfSideLength, -halfSideLength,  halfSideLength), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec2( 0.0f,  1.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3( 0.0f,  0.0f,  1.0f));
    vertices.emplace_back(glm::vec3(-halfSideLength, -halfSideLength, -halfSideLength), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec2( 0.0f,  0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3( 0.0f,  0.0f,  1.0f));
    vertices.emplace_back(glm::vec3( halfSideLength, -halfSideLength, -halfSideLength), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec2( 1.0f,  0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3( 0.0f,  0.0f,  1.0f));
    
    vertices.emplace_back(glm::vec3( halfSideLength,  halfSideLength,  halfSideLength), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec2( 1.0f,  1.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3( 0.0f,  1.0f,  0.0f));    // Front face.
    vertices.emplace_back(glm::vec3(-halfSideLength,  halfSideLength,  halfSideLength), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec2( 0.0f,  1.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3( 0.0f,  1.0f,  0.0f));
    vertices.emplace_back(glm::vec3(-halfSideLength, -halfSideLength,  halfSideLength), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec2( 0.0f,  0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3( 0.0f,  1.0f,  0.0f));
    vertices.emplace_back(glm::vec3( halfSideLength, -halfSideLength,  halfSideLength), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec2( 1.0f,  0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3( 0.0f,  1.0f,  0.0f));
    
    vertices.emplace_back(glm::vec3(-halfSideLength,  halfSideLength, -halfSideLength), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec2( 1.0f,  1.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3( 0.0f,  1.0f,  0.0f));    // Back face.
    vertices.emplace_back(glm::vec3( halfSideLength,  halfSideLength, -halfSideLength), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec2( 0.0f,  1.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3( 0.0f,  1.0f,  0.0f));
    vertices.emplace_back(glm::vec3( halfSideLength, -halfSideLength, -halfSideLength), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec2( 0.0f,  0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3( 0.0f,  1.0f,  0.0f));
    vertices.emplace_back(glm::vec3(-halfSideLength, -halfSideLength, -halfSideLength), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec2( 1.0f,  0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3( 0.0f,  1.0f,  0.0f));
    
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
        glm::vec3 position;
        position.z = radius * sin(stackAngle);
        
        for (int j = 0; j <= numSectors; ++j) {
            float sectorAngle = j * sectorStep;    // Go from 0 to 2PI.
            
            position.x = rCosTheta * cos(sectorAngle);
            position.y = rCosTheta * sin(sectorAngle);
            glm::vec3 normal = position / radius;
            glm::vec3 tangent(-sin(sectorAngle), cos(sectorAngle), 0.0f);
            glm::vec3 bitangent = glm::normalize(glm::cross(normal, tangent));
            vertices.emplace_back(position, normal, glm::vec2(static_cast<float>(j) / numSectors, 1.0f - static_cast<float>(i) / numStacks), tangent, bitangent);
        }
    }
    
    vector<unsigned int> indices;
    indices.reserve(numSectors * (numStacks - 1) * 6);
    for (int i = 0; i < numStacks; ++i) {
        int index1 = i * (numSectors + 1);    // Index of current stack.
        int index2 = index1 + numSectors + 1;    // Index of next stack.
        
        for (int j = 0; j < numSectors; ++j) {
            if (i != 0) {    // Skip first stacks.
                indices.push_back(index1 + 1);
                indices.push_back(index1);
                indices.push_back(index2);
            }
            
            if (i != numStacks - 1) {    // Skip last stacks.
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
    glBindVertexArray(vertexArrayHandle_);
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

void Mesh::draw(const Shader& shader, const glm::mat4& modelMtx) const {
    for (const Texture& t : textures_) {
        glActiveTexture(GL_TEXTURE0 + t.index);
        glBindTexture(GL_TEXTURE_2D, t.handle);
    }
    drawGeometry(shader, modelMtx);
}

void Mesh::drawGeometry() const {
    glBindVertexArray(vertexArrayHandle_);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices_.size()), GL_UNSIGNED_INT, 0);
}

void Mesh::drawGeometry(const Shader& shader, const glm::mat4& modelMtx) const {
    shader.setMat4("modelMtx", modelMtx);
    glBindVertexArray(vertexArrayHandle_);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices_.size()), GL_UNSIGNED_INT, 0);
}

void Mesh::drawInstanced(const Shader& shader, unsigned int count) const {
    for (const Texture& t : textures_) {
        glActiveTexture(GL_TEXTURE0 + t.index);
        glBindTexture(GL_TEXTURE_2D, t.handle);
    }
    drawGeometryInstanced(shader, count);
}

void Mesh::drawGeometryInstanced(const Shader& shader, unsigned int count) const {
    glBindVertexArray(vertexArrayHandle_);
    glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(indices_.size()), GL_UNSIGNED_INT, 0, count);
}

void Mesh::generateBuffers() {
    assert(vertexArrayHandle_ == 0);
    glGenVertexArrays(1, &vertexArrayHandle_);
    glBindVertexArray(vertexArrayHandle_);
    
    glGenBuffers(1, &vertexBufferHandle_);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferHandle_);
    
    glGenBuffers(1, &elementBufferHandle_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferHandle_);
}
