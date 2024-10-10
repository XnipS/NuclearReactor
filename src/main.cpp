#include <SDL.h>

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <thread>

#include "../include/core.h"
#include "../include/fluidEngine.h"
#include "../include/renderEngine.h"
#include "../include/soundMixer.h"
#include "SDL_keyboard.h"
#include "SDL_scancode.h"

renderEngine* render = nullptr;
fluidEngine* fluid = nullptr;
soundMixer* sound = nullptr;

Uint32 frameStart;
int currentTickTime;
std::vector<CircleData> reactorMaterial;
std::vector<CircleData> neutrons;
std::vector<RectangleData> reactorWater;
std::vector<RectangleData> reactorRod;
// Entrypoint
int main(int argc, char* args[])
{
    // Engines
    render = new renderEngine();
    fluid = new fluidEngine();
    sound = new soundMixer();

    // Start
    sound->InitMixer();
    int snd = sound->LoadSound("sound.wav");
    int geigerSnd = sound->LoadSound("geiger.wav");
    sound->PlaySound(snd);
    // Old size 1280 * 720
    render->Initialise("Nuclear Reactor Simulator", 1280 * 1.3, 720 * 1.3);
    fluid->Start(render);
    render->LinkSettings(&fluid->settings);
    fluid->settings.stats.ZeroGraph();
    // Spawn initial reactor
    for (int x = 0; x < NR_SIZE_X; x++) {
        for (int y = 0; y < NR_SIZE_Y; y++) {
            fluid->AddWater(x, y);
            if (RandomRange(0, 1.0) < NR_ENRICHMENT) {
                fluid->AddReactorMaterial(x, y, 1);
            } else {
                fluid->AddReactorMaterial(x, y, 0);
            }
        }
    }
    // Spawn rods
    fluid->AddControlRod(4, 50);
    fluid->AddControlRod(8, 100); // Static
    fluid->AddControlRod(12, 50);
    fluid->AddControlRod(16, 100); // Static
    fluid->AddControlRod(20, 50);
    fluid->AddControlRod(24, 100); // Static
    fluid->AddControlRod(28, 50);
    fluid->AddControlRod(32, 100); // Static
    fluid->AddControlRod(36, 50);

    render->LinkReactorMaterials(&reactorMaterial);
    render->LinkNeutrons(&neutrons);
    render->LinkReactorWater(&reactorWater);
    render->LinkReactorRod(&reactorRod);

    // Tick loop
    while (render->Running()) {
        // Start tick time
        frameStart = SDL_GetTicks();

        // Update & render
        std::thread fluidThread(&fluidEngine::Update, fluid);
        fluid->LinkReactorMaterialToMain(&reactorMaterial);
        fluid->LinkNeutronsToMain(&neutrons);
        fluid->LinkReactorWaterToMain(&reactorWater);
        fluid->LinkReactorRodToMain(&reactorRod);
        if (render->AddNetron() > 0) {
            for (int i = 0; i < render->AddNetron(); i++) {
                fluid->AddNeutron(RandomRange(0, NR_SIZE_X), RandomRange(0, NR_SIZE_Y));
            }
        }
        if (render->ClearNeutrons()) {
            fluid->ClearNeutrons();
        }

        fluid->SetControlRodHeight(0, fluid->settings.rodHeight_1);
        fluid->SetControlRodHeight(2, fluid->settings.rodHeight_2);
        fluid->SetControlRodHeight(4, fluid->settings.rodHeight_3);
        fluid->SetControlRodHeight(6, fluid->settings.rodHeight_4);
        fluid->SetControlRodHeight(8, fluid->settings.rodHeight_5);

        render->Update();
        render->Render();

        fluidThread.join();
        if (fluid->isPlayingSound) {
            sound->PlaySound(geigerSnd);
            fluid->isPlayingSound = false;
        }

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
    sound->QuitMixer();
    render->Clean();
    return 0;
}