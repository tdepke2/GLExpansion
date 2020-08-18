#include "Renderer.h"
#include "World.h"
#include <chrono>
#include <iostream>

using namespace std;

int main(int argc, char** argv) {
    cout << "Initializing setup...\n";
    mt19937 randNumGenerator;
    randNumGenerator.seed(static_cast<unsigned long>(chrono::high_resolution_clock::now().time_since_epoch().count()));
    Renderer renderer(&randNumGenerator);
    World world;
    cout << "Setup complete.\n";
    
    try {
        while (renderer.getState() != Renderer::Exiting) {
            renderer.beginFrame(world);
            renderer.drawShadowMaps(world);
            renderer.geometryPass(world);
            renderer.applySSAO();
            renderer.lightingPass();
            renderer.drawLamps(world);
            renderer.drawSkybox();
            renderer.applyBloom();
            renderer.drawPostProcessing();
            renderer.drawGUI();
            renderer.endFrame();
        }
    } catch (exception& ex) {
        renderer.setState(Renderer::Exiting);
        cout << "\n****************************************************\n";
        cout << "* A fatal error has occurred, terminating program. *\n";
        cout << "****************************************************\n";
        cout << "Error: " << ex.what() << "\n";
        cout << "(Press enter)\n";
        cin.get();
        return -1;
    }
    
    return 0;
}
