#ifndef MODEL_STATIC_H_
#define MODEL_STATIC_H_

#include "Mesh.h"
#include "ModelAbstract.h"

using namespace std;

class ModelStatic : public ModelAbstract {
    public:
    ModelStatic();
    ModelStatic(const string& filename);
    void loadFile(const string& filename);
    
    private:
    void processNode(aiNode* node, const aiScene* scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
};

#endif
