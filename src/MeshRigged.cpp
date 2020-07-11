#include "MeshRigged.h"
#include "Simulator.h"
#include <cassert>
#include <cmath>
#include <utility>

#include <iostream>

void MeshRigged::Vertex::addBone(unsigned int id, float w) {
    if (weight.x == 0.0f) {
        bone.x = id;
        weight.x = w;
    } else if (weight.y == 0.0f) {
        bone.y = id;
        weight.y = w;
    } else if (weight.z == 0.0f) {
        bone.z = id;
        weight.z = w;
    } else if (weight.w == 0.0f) {
        bone.w = id;
        weight.w = w;
    } else {
        cout << "Warn: Vertex with more than four bone attachments not allowed.\n";
    }
}

MeshRigged::MeshRigged() {
    _vertexArrayHandle = 0;
    _vertexBufferHandle = 0;
    _elementBufferHandle = 0;
}

MeshRigged::MeshRigged(vector<Vertex>&& vertices, vector<unsigned int>&& indices, vector<Texture>&& textures) {
    _vertexArrayHandle = 0;
    _vertexBufferHandle = 0;
    _elementBufferHandle = 0;
    generateMesh(move(vertices), move(indices), move(textures));
}

MeshRigged::~MeshRigged() {
    glDeleteVertexArrays(1, &_vertexArrayHandle);
    glDeleteBuffers(1, &_vertexBufferHandle);
    glDeleteBuffers(1, &_elementBufferHandle);
}

MeshRigged::MeshRigged(MeshRigged&& mesh) : _vertexArrayHandle(mesh._vertexArrayHandle), _vertexBufferHandle(mesh._vertexBufferHandle), _elementBufferHandle(mesh._elementBufferHandle) {
    vertices = move(mesh.vertices);
    indices = move(mesh.indices);
    textures = move(mesh.textures);
    mesh._vertexArrayHandle = 0;
    mesh._vertexBufferHandle = 0;
    mesh._elementBufferHandle = 0;
}

MeshRigged& MeshRigged::operator=(MeshRigged&& mesh) {
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

void MeshRigged::generateMesh(vector<Vertex>&& vertices, vector<unsigned int>&& indices, vector<Texture>&& textures) {
    assert(_vertexArrayHandle == 0);
    this->vertices = vertices;
    this->indices = indices;
    this->textures = textures;
    
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
    glEnableVertexAttribArray(Simulator::ATTRIBUTE_LOCATION_V_TANGENT);
    glVertexAttribPointer(Simulator::ATTRIBUTE_LOCATION_V_TANGENT, 3, GL_FLOAT, false, sizeof(Vertex), reinterpret_cast<void*>(sizeof(float) * 8));
    glEnableVertexAttribArray(Simulator::ATTRIBUTE_LOCATION_V_BITANGENT);
    glVertexAttribPointer(Simulator::ATTRIBUTE_LOCATION_V_BITANGENT, 3, GL_FLOAT, false, sizeof(Vertex), reinterpret_cast<void*>(sizeof(float) * 11));
    glEnableVertexAttribArray(Simulator::ATTRIBUTE_LOCATION_V_BONE);
    glVertexAttribIPointer(Simulator::ATTRIBUTE_LOCATION_V_BONE, 4, GL_UNSIGNED_INT, sizeof(Vertex), reinterpret_cast<void*>(sizeof(float) * 14));
    glEnableVertexAttribArray(Simulator::ATTRIBUTE_LOCATION_V_WEIGHT);
    glVertexAttribPointer(Simulator::ATTRIBUTE_LOCATION_V_WEIGHT, 4, GL_FLOAT, false, sizeof(Vertex), reinterpret_cast<void*>(sizeof(float) * 14 + sizeof(unsigned int) * 4));
}

void MeshRigged::draw() const {
    glBindVertexArray(_vertexArrayHandle);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
}

void MeshRigged::draw(const Shader& shader) const {
    for (unsigned int i = 0; i < textures.size(); ++i) {
        glActiveTexture(GL_TEXTURE0 + textures[i].index);    // ############################################################# Maybe there is no need to pass a shader?
        glBindTexture(GL_TEXTURE_2D, textures[i].handle);
    }
    draw();
}
