#include <SDL.h>

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <thread>

#include "../include/core.h"
#include "../include/fluidEngine.h"
#include "../include/renderEngine.h"

renderEngine* render = nullptr;
fluidEngine* fluid = nullptr;

Uint32 frameStart;
int currentTickTime;
std::vector<CircleData> reactorMaterial;
std::vector<CircleData> neutrons;

// Entrypoint
int main(int argc, char* args[])
{
    // Engines
    render = new renderEngine();
    fluid = new fluidEngine();

    // Start
    render->Initialise("Nuclear Reactor Simulator", 1280, 720);
    fluid->Start(render);
    render->LinkSettings(&fluid->settings);

    // Spawn initial reactor material
    for (int x = 0; x < NR_SIZE_X; x++) {
        for (int y = 0; y < NR_SIZE_Y; y++) {
            if (RandomRange(0, 1.0) < NR_ENRICHMENT) {
                fluid->AddReactorMaterial(x, y, 1);
            } else {
                fluid->AddReactorMaterial(x, y, 0);
                // fluid->AddNeutron(x, y);
            }
            // fluid->AddReactorMaterial(x, y, 1);
        }
    }

    render->LinkReactorMaterials(&reactorMaterial);
    render->LinkNeutrons(&neutrons);

    // Tick loop
    while (render->Running()) {
        // Start tick time
        frameStart = SDL_GetTicks();

        // Update & render
        std::thread fluidThread(&fluidEngine::Update, fluid);
        fluid->LinkReactorMaterialToMain(&reactorMaterial);
        fluid->LinkNeutronsToMain(&neutrons);
        if (render->AddNetron() > 0) {
            for (int i = 0; i < render->AddNetron(); i++) {
                fluid->AddNeutron(RandomRange(0, NR_SIZE_X), RandomRange(0, NR_SIZE_Y));
            }
        }
        if (render->ClearNeutrons()) {
            fluid->ClearNeutrons();
        }
        // render->val_totalSand = fluid->SandCount();
        render->Update();
        render->Render();

        fluidThread.join();

        // Check for delays
        currentTickTime = SDL_GetTicks() - frameStart;
        if (NE_TICKRATE_TIME > currentTickTime) {
            SDL_Delay(NE_TICKRATE_TIME - currentTickTime);
        } else {
            std::cout << "Tickrate lagging: ";
            std::cout << (currentTickTime - NE_TICKRATE_TIME);
            std::cout << "ms behind!" << std::endl;
        }
    }
    // Clean
    render->Clean();
    return 0;
}