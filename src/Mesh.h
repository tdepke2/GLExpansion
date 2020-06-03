#ifndef _MESH_H
#define _MESH_H

#include "Shader.h"
#include <string>
#include <vector>

using namespace std;

class Mesh {
    public:
    struct Vertex {
        float vx, vy, vz;
        float nx, ny, nz;
        float s, t;
        
        Vertex() {}
        Vertex(float vx, float vy, float vz, float nx, float ny, float nz, float s, float t) : vx(vx), vy(vy), vz(vz), nx(nx), ny(ny), nz(nz), s(s), t(t) {}
    };
    
    struct Texture {
        unsigned int id;
        string uniformName;
        
        Texture() {}
        Texture(unsigned int id, const string& uniformName) : id(id), uniformName(uniformName) {}
    };
    
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    vector<Texture> textures;
    
    Mesh();
    Mesh(vector<Vertex>&& vertices, vector<unsigned int>&& indices);
    Mesh(vector<Vertex>&& vertices, vector<unsigned int>&& indices, vector<Texture>&& textures);
    ~Mesh();
    Mesh(const Mesh& mesh) = delete;
    Mesh& operator=(const Mesh& mesh) = delete;
    Mesh(Mesh&& mesh);
    Mesh& operator=(Mesh&& mesh);
    void generateMesh(vector<Vertex>&& vertices, vector<unsigned int>&& indices);
    void generateMesh(vector<Vertex>&& vertices, vector<unsigned int>&& indices, vector<Texture>&& textures);
    void generateCube(float sideLength = 1.0f);
    void generateSphere(float radius = 1.0f, int numSectors = 32, int numStacks = 16);
    void applyInstanceBuffer(unsigned int startIndex) const;
    void draw() const;
    void draw(const Shader& shader) const;
    void drawInstanced(unsigned int count) const;
    void drawInstanced(const Shader& shader, unsigned int count) const;
    
    private:
    unsigned int _vertexArrayHandle, _vertexBufferHandle, _elementBufferHandle;
};

#endif
