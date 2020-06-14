#ifndef _MESH_H
#define _MESH_H

#include "Shader.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <vector>

using namespace std;

class Mesh {
    public:
    struct Vertex {
        float x, y, z;       // Position.
        float nx, ny, nz;    // Normal.
        float s, t;          // Texture coords.
        float tx, ty, tz;    // Tangent.
        float bx, by, bz;    // Bitangent.
        
        Vertex() {}
        Vertex(const glm::vec3& p, const glm::vec3& n, const glm::vec2& c) : x(p.x), y(p.y), z(p.z), nx(n.x), ny(n.y), nz(n.z), s(c.s), t(c.t), tx(0.0f), ty(0.0f), tz(0.0f), bx(0.0f), by(0.0f), bz(0.0f) {}
        Vertex(const glm::vec3& p, const glm::vec3& n, const glm::vec2& c, const glm::vec3& t, const glm::vec3& b) : x(p.x), y(p.y), z(p.z), nx(n.x), ny(n.y), nz(n.z), s(c.s), t(c.t), tx(t.x), ty(t.y), tz(t.z), bx(b.x), by(b.y), bz(b.z) {}
    };
    
    struct Texture {
        unsigned int handle;
        unsigned int index;
        
        Texture() {}
        Texture(unsigned int handle, unsigned int index) : handle(handle), index(index) {}
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
