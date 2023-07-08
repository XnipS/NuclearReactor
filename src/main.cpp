#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_timer.h>

#include <cstdio>
#include <iostream>

#include "../include/core.h"
#include "../include/fluidEngine.h"
#include "../include/renderEngine.h"

renderEngine *engine = nullptr;
fluidEngine *fengine = nullptr;
Uint32 frameStart;
int currentTickTime;

int main(int argc, char *args[]) {
  engine = new renderEngine();
  fengine = new fluidEngine();

  engine->Initialise("Fluidised Bed Engine", 800, 640);
  fengine->Start();
  fengine->AddSandAtPos(0, 0);

  while (engine->Running()) {
    frameStart = SDL_GetTicks();
    fengine->Update();
    float pixels[500 * 500 * 3];
    float *test = fengine->SandToColour(pixels);
    engine->UpdateImage(test);
    engine->Update();
    engine->Render();

    currentTickTime = SDL_GetTicks() - frameStart;
    if (RE_TICKRATE_TIME > currentTickTime) {
      SDL_Delay(RE_TICKRATE_TIME - currentTickTime);
    } else {
      std::cout << "Tickrate lagging: ";
      std::cout << (currentTickTime - RE_TICKRATE_TIME);
      std::cout << "ms behind!" << std::endl;
    }
  }

  engine->Clean();
  return 0;
}