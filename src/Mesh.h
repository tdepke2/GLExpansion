#ifndef MESH_H_
#define MESH_H_

class Shader;

#include "DrawableInterface.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

using namespace std;

class Mesh : public DrawableInterface {
    public:
    struct Vertex {
        glm::vec3 pos;    // Position.
        glm::vec3 norm;    // Normal.
        glm::vec2 tex;    // Texture coords.
        glm::vec3 tan;    // Tangent.
        glm::vec3 bitan;    // Bitangent.
        
        Vertex() {}
        Vertex(const glm::vec3& pos, const glm::vec2& tex) : pos(pos), norm(0.0f), tex(tex), tan(0.0f), bitan(0.0f) {}
        Vertex(const glm::vec3& pos, const glm::vec3& norm, const glm::vec2& tex, const glm::vec3& tan, const glm::vec3& bitan) : pos(pos), norm(norm), tex(tex), tan(tan), bitan(bitan) {}
    };
    
    struct VertexBone {
        glm::vec3 pos;    // Position.
        glm::vec3 norm;    // Normal.
        glm::vec2 tex;    // Texture coords.
        glm::vec3 tan;    // Tangent.
        glm::vec3 bitan;    // Bitangent.
        uint32_t bone;    // Bone ID.
        glm::vec4 weight;    // Bone weight.
        
        VertexBone() {}
        VertexBone(const glm::vec3& pos, const glm::vec3& norm, const glm::vec2& tex, const glm::vec3& tan, const glm::vec3& bitan) : pos(pos), norm(norm), tex(tex), tan(tan), bitan(bitan), bone(0), weight(0.0f) {}
        void addBone(uint8_t id, float w);
    };
    
    struct Texture {
        unsigned int handle;
        unsigned int index;
        
        Texture() {}
        Texture(unsigned int handle, unsigned int index) : handle(handle), index(index) {}
    };
    
    vector<glm::vec3> vertexPositions_;
    vector<unsigned int> indices_;
    vector<Texture> textures_;
    
    Mesh();
    Mesh(vector<Vertex>&& vertices, vector<unsigned int>&& indices);
    Mesh(vector<Vertex>&& vertices, vector<unsigned int>&& indices, vector<Texture>&& textures);
    Mesh(vector<VertexBone>&& vertices, vector<unsigned int>&& indices);
    Mesh(vector<VertexBone>&& vertices, vector<unsigned int>&& indices, vector<Texture>&& textures);
    ~Mesh();
    Mesh(const Mesh& mesh) = delete;
    Mesh& operator=(const Mesh& mesh) = delete;
    Mesh(Mesh&& mesh);
    Mesh& operator=(Mesh&& mesh);
    void bindVAO() const;
    void generateMesh(vector<Vertex>&& vertices, vector<unsigned int>&& indices);    // Sets up the mesh buffers and vertex array given the Vertex data.
    void generateMesh(vector<Vertex>&& vertices, vector<unsigned int>&& indices, vector<Texture>&& textures);
    void generateMesh(vector<VertexBone>&& vertices, vector<unsigned int>&& indices);     // Sets up the mesh buffers and vertex array given the VertexBone data.
    void generateMesh(vector<VertexBone>&& vertices, vector<unsigned int>&& indices, vector<Texture>&& textures);
    void generateCube(float sideLength = 1.0f);
    void generateSphere(float radius = 1.0f, int numSectors = 32, int numStacks = 16);
    void generateCylinder(float radiusBase = 1.0f, float radiusTop = 1.0f, float height = 2.0f, int numSectors = 32, int numStacks = 1, bool originAtBase = false);
    void applyMat4InstanceBuffer(unsigned int startIndex, unsigned int stride, size_t offset) const;    // Binds the vertex array and sets attributes for the currently bound buffer (buffer should contain mat4 data). This uses attributes startIndex to startIndex + 3.
    void draw(const Shader& shader, const glm::mat4& modelMtx) const;
    void drawGeometry() const;
    void drawGeometry(const Shader& shader, const glm::mat4& modelMtx) const;
    void drawInstanced(unsigned int count) const;
    void drawGeometryInstanced(unsigned int count) const;
    
    private:
    unsigned int vertexArrayHandle_, vertexBufferHandle_, elementBufferHandle_;
    
    void generateBuffers();
};

#endif
