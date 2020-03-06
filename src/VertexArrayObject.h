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
    static constexpr unsigned int NUM_BUFFERS = 1;
    VertexArrayObject();
    ~VertexArrayObject();
    void generateVertices(vector<VertexAttributes>&& _vertices);
    void generateCube(float sideLength = 1.0f);
    void generateSphere();
    void draw();
    
    private:
    unsigned int _vaoHandle;
    unsigned int _vboHandles[NUM_BUFFERS];
    vector<VertexAttributes> _vertices;
};

#endif
