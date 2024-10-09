#include "../include/soundMixer.h"
#include "SDL.h"
#include <SDL_mixer.h>
#include <cstddef>
#include <cstdio>
#include <vector>

std::vector<Mix_Chunk*> sounds;

soundMixer::soundMixer() {};
soundMixer::~soundMixer() {};

int soundMixer::LoadSound(const char* filename)
{
    Mix_Chunk* m = NULL;
    m = Mix_LoadWAV(filename);
    if (m == NULL) {
        printf("Failed to load sound!");
        return -1;
    }
    sounds.push_back(m);
    return sounds.size() - 1;
}

void soundMixer::SetVolume(int v)
{
    volume = (MIX_MAX_VOLUME * v) / 100;
}

int soundMixer::PlaySound(int s)
{
    Mix_Volume(-1, volume);
    Mix_PlayChannel(-1, sounds[s], 0);
    return 0;
}

int soundMixer::InitMixer()
{
    Mix_Init(MIX_INIT_MP3);
    SDL_Init(SDL_INIT_AUDIO);
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("Failed initialise mixer!");
        return -1;
    }
    SetVolume(80);
    return 0;
}
int soundMixer::QuitMixer()
{
    for (int i = 0; i < sounds.size(); i++) {
        Mix_FreeChunk(sounds[i]);
        sounds[i] = NULL;
    }
    Mix_Quit();
}