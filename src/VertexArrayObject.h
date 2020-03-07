#ifndef _VERTEX_ARRAY_OBJECT
#define _VERTEX_ARRAY_OBJECT

#include <vector>

using namespace std;

struct VertexAttributes {
    float vx, vy, vz;
    float nx, ny, nz;
    float s, t;
    
    VertexAttributes() {}
    VertexAttributes(float vx, float vy, float vz, float nx, float ny, float nz, float s, float t) : vx(vx), vy(vy), vz(vz), nx(nx), ny(ny), nz(nz), s(s), t(t) {}
};

class VertexArrayObject {
    public:
    VertexArrayObject();
    ~VertexArrayObject();
    void generateVertices(vector<VertexAttributes>&& _vertices);
    void generateVertices(vector<VertexAttributes>&& _vertices, vector<unsigned int>&& _indices);
    void generateCube(float sideLength = 1.0f);
    void generateSphere(float radius = 1.0f, int numSectors = 32, int numStacks = 16);
    void draw();
    
    private:
    unsigned int _vertexArrayHandle, _vertexBufferHandle, _elementBufferHandle;
    vector<VertexAttributes> _vertices;
    vector<unsigned int> _indices;
};

#endif
