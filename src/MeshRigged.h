#ifndef _MESH_RIGGED_H
#define _MESH_RIGGED_H

#include "Shader.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <vector>

using namespace std;

class MeshRigged {
    public:
    //static constexpr int MAX_BONES_PER_VERTEX = 4;
    
    struct Vertex {
        glm::vec3 pos;    // Position.
        glm::vec3 norm;    // Normal.
        glm::vec2 tex;    // Texture coords.
        glm::vec3 tan;    // Tangent.
        glm::vec3 bitan;    // Bitangent.
        glm::uvec4 bone;    // Bone ID.
        glm::vec4 weight;    // Bone weight.
        //uint8_t bone[MAX_BONES_PER_VERTEX]; ###########################################################
        //float weight[MAX_BONES_PER_VERTEX];
        
        Vertex() {}
        Vertex(const glm::vec3& pos, const glm::vec3& norm, const glm::vec2& tex, const glm::vec3& tan, const glm::vec3& bitan) : pos(pos), norm(norm), tex(tex), tan(tan), bitan(bitan), bone(glm::uvec4(0)), weight(glm::vec4(0.0f)) {
            /*for (int i = 0; i < MAX_BONES_PER_VERTEX; ++i) {
                this->bone[i] = bone[i];
                this->weight[i] = weight[i];
            }*/
        }
        void addBone(unsigned int id, float w);
    };
    
    struct Texture {
        unsigned int handle;
        unsigned int index;
        
        Texture() {}
        Texture(unsigned int handle, unsigned int index) : handle(handle), index(index) {}
    };
    
    vector<Vertex> vertices;    // Probably don't want to store these or the indices :/ #################################################################################################
    vector<unsigned int> indices;
    vector<Texture> textures;
    
    MeshRigged();
    MeshRigged(vector<Vertex>&& vertices, vector<unsigned int>&& indices, vector<Texture>&& textures);
    ~MeshRigged();
    MeshRigged(const MeshRigged& mesh) = delete;
    MeshRigged& operator=(const MeshRigged& mesh) = delete;
    MeshRigged(MeshRigged&& mesh);
    MeshRigged& operator=(MeshRigged&& mesh);
    void generateMesh(vector<Vertex>&& vertices, vector<unsigned int>&& indices, vector<Texture>&& textures);
    void draw() const;
    void draw(const Shader& shader) const;
    
    private:
    unsigned int _vertexArrayHandle, _vertexBufferHandle, _elementBufferHandle;
};

#endif
