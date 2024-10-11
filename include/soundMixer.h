#include <SDL_mixer.h>

#include <cstdio>

class soundMixer {
public:
    soundMixer();
    ~soundMixer();
    int LoadSound(const char* filename);

    void SetVolume(int v);

    int PlaySound(int s);

    int InitMixer();
    void QuitMixer();

private:
    int volume;
};