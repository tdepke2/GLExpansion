#include "Mesh.h"
#include "Renderer.h"
#include "Shader.h"
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

void Mesh::bindVAO() const {
    glBindVertexArray(vertexArrayHandle_);
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
    
    glEnableVertexAttribArray(Renderer::ATTRIBUTE_LOCATION_V_POSITION);    // Specify the position and stride for vertices, normals, and tex coords in the array.
    glVertexAttribPointer(Renderer::ATTRIBUTE_LOCATION_V_POSITION, 3, GL_FLOAT, false, sizeof(Vertex), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(Renderer::ATTRIBUTE_LOCATION_V_NORMAL);
    glVertexAttribPointer(Renderer::ATTRIBUTE_LOCATION_V_NORMAL, 3, GL_FLOAT, false, sizeof(Vertex), reinterpret_cast<void*>(sizeof(float) * 3));
    glEnableVertexAttribArray(Renderer::ATTRIBUTE_LOCATION_V_TEX_COORDS);
    glVertexAttribPointer(Renderer::ATTRIBUTE_LOCATION_V_TEX_COORDS, 2, GL_FLOAT, false, sizeof(Vertex), reinterpret_cast<void*>(sizeof(float) * 6));
    glEnableVertexAttribArray(Renderer::ATTRIBUTE_LOCATION_V_TANGENT);
    glVertexAttribPointer(Renderer::ATTRIBUTE_LOCATION_V_TANGENT, 3, GL_FLOAT, false, sizeof(Vertex), reinterpret_cast<void*>(sizeof(float) * 8));
    glEnableVertexAttribArray(Renderer::ATTRIBUTE_LOCATION_V_BITANGENT);
    glVertexAttribPointer(Renderer::ATTRIBUTE_LOCATION_V_BITANGENT, 3, GL_FLOAT, false, sizeof(Vertex), reinterpret_cast<void*>(sizeof(float) * 11));
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
    
    glEnableVertexAttribArray(Renderer::ATTRIBUTE_LOCATION_V_POSITION);    // Specify the position and stride for vertices, normals, and tex coords in the array.
    glVertexAttribPointer(Renderer::ATTRIBUTE_LOCATION_V_POSITION, 3, GL_FLOAT, false, sizeof(VertexBone), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(Renderer::ATTRIBUTE_LOCATION_V_NORMAL);
    glVertexAttribPointer(Renderer::ATTRIBUTE_LOCATION_V_NORMAL, 3, GL_FLOAT, false, sizeof(VertexBone), reinterpret_cast<void*>(sizeof(float) * 3));
    glEnableVertexAttribArray(Renderer::ATTRIBUTE_LOCATION_V_TEX_COORDS);
    glVertexAttribPointer(Renderer::ATTRIBUTE_LOCATION_V_TEX_COORDS, 2, GL_FLOAT, false, sizeof(VertexBone), reinterpret_cast<void*>(sizeof(float) * 6));
    glEnableVertexAttribArray(Renderer::ATTRIBUTE_LOCATION_V_TANGENT);
    glVertexAttribPointer(Renderer::ATTRIBUTE_LOCATION_V_TANGENT, 3, GL_FLOAT, false, sizeof(VertexBone), reinterpret_cast<void*>(sizeof(float) * 8));
    glEnableVertexAttribArray(Renderer::ATTRIBUTE_LOCATION_V_BITANGENT);
    glVertexAttribPointer(Renderer::ATTRIBUTE_LOCATION_V_BITANGENT, 3, GL_FLOAT, false, sizeof(VertexBone), reinterpret_cast<void*>(sizeof(float) * 11));
    glEnableVertexAttribArray(Renderer::ATTRIBUTE_LOCATION_V_BONE);
    glVertexAttribIPointer(Renderer::ATTRIBUTE_LOCATION_V_BONE, 1, GL_UNSIGNED_INT, sizeof(VertexBone), reinterpret_cast<void*>(sizeof(float) * 14));
    glEnableVertexAttribArray(Renderer::ATTRIBUTE_LOCATION_V_WEIGHT);
    glVertexAttribPointer(Renderer::ATTRIBUTE_LOCATION_V_WEIGHT, 4, GL_FLOAT, false, sizeof(VertexBone), reinterpret_cast<void*>(sizeof(float) * 14 + sizeof(unsigned int)));
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
    
    vector<Vertex> vertices;    // Based on implementation from http://www.songho.ca/opengl/gl_sphere.html#sphere
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
        unsigned int index1 = i * (numSectors + 1);    // Index of current stack.
        unsigned int index2 = index1 + numSectors + 1;    // Index of next stack.
        
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

void Mesh::generateCylinder(float radiusBase, float radiusTop, float height, int numSectors, int numStacks) {
    assert(numSectors >= 3 && numStacks >= 1);
    
    vector<Vertex> vertices;
    vertices.reserve((numSectors + 1) * (numStacks + 1) + (radiusTop != 0.0f ? numSectors + 1 : 0) + (radiusBase != 0.0f ? numSectors + 1 : 0));
    float sectorStep = 2.0f * glm::pi<float>() / numSectors;
    float zAngle = atan2(radiusBase - radiusTop, height);
    glm::vec3 position, normal, tangent, bitangent;
    for (int i = 0; i <= numStacks; ++i) {    // Add side vertices from top-to-bottom.
        position.z = height * 0.5f - static_cast<float>(i) / numStacks * height;
        normal.z = sin(zAngle);
        float r = radiusTop + static_cast<float>(i) / numStacks * (radiusBase - radiusTop);    // Lerp radiusBase and radiusTop to get radius of this slice.
        
        for (int j = 0; j <= numSectors; ++j) {
            float sectorAngle = j * sectorStep;    // Go from 0 to 2PI.
            
            position.x = r * cos(sectorAngle);
            position.y = r * sin(sectorAngle);
            normal.x = cos(sectorAngle) * cos(zAngle);
            normal.y = sin(sectorAngle) * cos(zAngle);
            tangent = glm::vec3(-sin(sectorAngle), cos(sectorAngle), 0.0f);
            bitangent = glm::normalize(glm::cross(normal, tangent));
            vertices.emplace_back(position, normal, glm::vec2(static_cast<float>(j) / numSectors, 1.0f - static_cast<float>(i) / numStacks), tangent, bitangent);
        }
    }
    
    unsigned int topVerticesStart = static_cast<unsigned int>(vertices.size());
    if (radiusTop != 0.0f) {
        position = glm::vec3(0.0f, 0.0f, height * 0.5f);    // Add top vertices.
        normal = glm::vec3(0.0f, 0.0f, 1.0f);
        tangent = glm::vec3(1.0f, 0.0f, 0.0f);
        bitangent = glm::normalize(glm::cross(normal, tangent));
        vertices.emplace_back(position, normal, glm::vec2(0.5f, 0.5f), tangent, bitangent);
        for (int i = 0; i < numSectors; ++i) {
            float sectorAngle = i * sectorStep;    // Go from 0 to 2PI.
            
            position = glm::vec3(cos(sectorAngle) * radiusTop, sin(sectorAngle) * radiusTop, height * 0.5f);
            normal = glm::vec3(0.0f, 0.0f, 1.0f);
            tangent = glm::vec3(1.0f, 0.0f, 0.0f);
            bitangent = glm::normalize(glm::cross(normal, tangent));
            vertices.emplace_back(position, normal, glm::vec2(cos(sectorAngle) * 0.5f + 0.5f, sin(sectorAngle) * 0.5f + 0.5f), tangent, bitangent);
        }
    }
    
    unsigned int baseVerticesStart = static_cast<unsigned int>(vertices.size());
    if (radiusBase != 0.0f) {
        position = glm::vec3(0.0f, 0.0f, -height * 0.5f);    // Add base vertices.
        normal = glm::vec3(0.0f, 0.0f, -1.0f);
        tangent = glm::vec3(-1.0f, 0.0f, 0.0f);
        bitangent = glm::normalize(glm::cross(normal, tangent));
        vertices.emplace_back(position, normal, glm::vec2(0.5f, 0.5f), tangent, bitangent);
        for (int i = 0; i < numSectors; ++i) {
            float sectorAngle = i * sectorStep;    // Go from 0 to 2PI.
            
            position = glm::vec3(cos(sectorAngle) * radiusBase, sin(sectorAngle) * radiusBase, -height * 0.5f);
            normal = glm::vec3(0.0f, 0.0f, -1.0f);
            tangent = glm::vec3(-1.0f, 0.0f, 0.0f);
            bitangent = glm::normalize(glm::cross(normal, tangent));
            vertices.emplace_back(position, normal, glm::vec2(-cos(sectorAngle) * 0.5f + 0.5f, sin(sectorAngle) * 0.5f + 0.5f), tangent, bitangent);
        }
    }
    
    vector<unsigned int> indices;
    indices.reserve(numSectors * numStacks * 6 + (radiusTop != 0.0f ? numSectors * 3 : 0) + (radiusBase != 0.0f ? numSectors * 3 : 0));
    for (int i = 0; i < numStacks; ++i) {    // Add side indices.
        unsigned int index1 = i * (numSectors + 1);    // Index of current stack.
        unsigned int index2 = index1 + numSectors + 1;    // Index of next stack.
        
        for (int j = 0; j < numSectors; ++j) {
            indices.push_back(index1 + 1);
            indices.push_back(index1);
            indices.push_back(index2);
            
            indices.push_back(index2);
            indices.push_back(index2 + 1);
            indices.push_back(index1 + 1);
            
            ++index1;
            ++index2;
        }
    }
    
    if (radiusTop != 0.0f) {
        for (int i = 0; i < numSectors; ++i) {    // Add top indices.
            if (i < numSectors - 1) {
                indices.push_back(topVerticesStart);
                indices.push_back(topVerticesStart + i + 1);
                indices.push_back(topVerticesStart + i + 2);
            } else {
                indices.push_back(topVerticesStart);
                indices.push_back(topVerticesStart + i + 1);
                indices.push_back(topVerticesStart + 1);
            }
        }
    }
    
    if (radiusBase != 0.0f) {
        for (int i = 0; i < numSectors; ++i) {    // Add base indices.
            if (i < numSectors - 1) {
                indices.push_back(baseVerticesStart);
                indices.push_back(baseVerticesStart + i + 2);
                indices.push_back(baseVerticesStart + i + 1);
            } else {
                indices.push_back(baseVerticesStart);
                indices.push_back(baseVerticesStart + 1);
                indices.push_back(baseVerticesStart + i + 1);
            }
        }
    }
    
    generateMesh(move(vertices), move(indices));
}

void Mesh::applyMat4InstanceBuffer(unsigned int startIndex, unsigned int stride, size_t offset) const {
    glBindVertexArray(vertexArrayHandle_);
    glEnableVertexAttribArray(startIndex);
    glVertexAttribPointer(startIndex, 4, GL_FLOAT, false, stride, reinterpret_cast<void*>(offset));
    glVertexAttribDivisor(startIndex, 1);
    glEnableVertexAttribArray(startIndex + 1);
    glVertexAttribPointer(startIndex + 1, 4, GL_FLOAT, false, stride, reinterpret_cast<void*>(offset + 1 * sizeof(glm::vec4)));
    glVertexAttribDivisor(startIndex + 1, 1);
    glEnableVertexAttribArray(startIndex + 2);
    glVertexAttribPointer(startIndex + 2, 4, GL_FLOAT, false, stride, reinterpret_cast<void*>(offset + 2 * sizeof(glm::vec4)));
    glVertexAttribDivisor(startIndex + 2, 1);
    glEnableVertexAttribArray(startIndex + 3);
    glVertexAttribPointer(startIndex + 3, 4, GL_FLOAT, false, stride, reinterpret_cast<void*>(offset + 3 * sizeof(glm::vec4)));
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

void Mesh::drawInstanced(unsigned int count) const {
    for (const Texture& t : textures_) {
        glActiveTexture(GL_TEXTURE0 + t.index);
        glBindTexture(GL_TEXTURE_2D, t.handle);
    }
    drawGeometryInstanced(count);
}

void Mesh::drawGeometryInstanced(unsigned int count) const {
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
