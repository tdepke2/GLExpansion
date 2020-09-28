#ifndef MODEL_STATIC_H_
#define MODEL_STATIC_H_

#include "ModelAbstract.h"

using namespace std;

class ModelStatic : public ModelAbstract {
    public:
    ModelStatic();
    ModelStatic(const string& filename, unordered_map<string, Animation>* animations = nullptr);
    void loadFile(const string& filename, unordered_map<string, Animation>* animations = nullptr);
    
    private:
    void processNode(aiNode* node, glm::mat4 combinedTransform, const aiScene* scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
};

#endif
