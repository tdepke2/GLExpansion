#include "World.h"

World::World() {
    lightCube.generateCube(0.2f);
    cube1.generateCube();
    sphere1.generateSphere();
    
    sceneTest.loadFile("models/boot_camp/boot_camp.obj");
    sceneTestTransform.setScale(glm::vec3(0.025f, 0.025f, 0.025f));
    sceneTestTransform.setPitchYawRoll(glm::vec3(-glm::pi<float>() / 2.0f, 0.0f, 0.0f));
    
    modelTest.loadFile("models/bob_lamp_update/bob_lamp_update.md5mesh");
    //modelTest.loadFile("models/hellknight/hellknight.md5mesh");
    //modelTest.loadFile("models/spaceship/Intergalactic Spaceship_Blender_2.8_Packed textures.dae");
    modelTestTransform.setScale(glm::vec3(0.3f));
    modelTestTransform.setPosition(glm::vec3(0.0f, 0.0f, 2.0f));
}

World::~World() {
    
}
