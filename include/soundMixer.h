
class soundMixer {
public:
    soundMixer();
    ~soundMixer();
    int LoadSound(const char* filename);

    void SetVolume(int v) ;

    int PlaySound(int s) ;

    int InitMixer() ;
    int QuitMixer() ;

private:
    int volume;
};