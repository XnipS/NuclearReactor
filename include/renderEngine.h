#pragma once

#include <string>
#include <vector>

#include "VectorMath.h"

struct Colour3 {
    float r = 0;
    float b = 0;
    float g = 0;
    Colour3(float red, float green, float blue)
    {
        r = red;
        b = blue;
        g = green;
    }
};

struct ParticleStats {
    double pos_x;
    double pos_y;

    double vel_x;
    double vel_y;
};

struct ReactorSettings {
    int fissionNeutronCount = 3;
    float fissionNeutronSpeed = 200;
    float decayChance = 0.001;
    float regenerateChance = 0.001;
    float heatDissipate = 10;
    float heatTransfer = 10;
};

struct CircleData {
    VM::Vector2 position = VM::Vector2(0, 0);
    float radius = 1;
    int colourID = 0;
    CircleData(VM::Vector2 pos, float rad, int colour = 0)
    {
        position = pos;
        radius = rad;
        colourID = colour;
    };
};

struct RectangleData {
    VM::Vector2 position = VM::Vector2(0, 0);
    VM::Vector2 size = VM::Vector2(0, 0);

    int colourID = 0;
    RectangleData(VM::Vector2 pos, VM::Vector2 scale, int colour = 0)
    {
        position = pos;
        size = scale;
        colourID = colour;
    };
};

class renderEngine {
public:
    renderEngine();
    ~renderEngine();

    void Initialise(const char* title, int w, int h);
    void LinkSettings(ReactorSettings* set) { settings = set; };
    void UpdateImage(float* colours);
    void LinkReactorMaterials(std::vector<CircleData>* newPos);
    void LinkNeutrons(std::vector<CircleData>* newPos);
    void LinkReactorWater(std::vector<RectangleData>* newPos);
    void FloodImage(Colour3 col);
    void Update();
    void Render();
    void Clean();
    bool Running() { return isRunning; };
    std::vector<std::string> currentDebugInfo;
    int val_totalSand;
    int AddNetron() { return addNeutrons; };
    bool ClearNeutrons() { return clearAllNeutrons; };

private:
    ReactorSettings* settings;
    int tick = 0;
    bool isRunning;
    int addNeutrons = 0;
    bool clearAllNeutrons = false;
};

inline double RandomRange(double fMin, double fMax)
{
    double f = (double)rand() / RAND_MAX;
    return fMin + f * (fMax - fMin);
};

inline VM::Vector2 RandomUnitVector()
{
    double theta = RandomRange(0, 2 * M_PI);

    return *new VM::Vector2(cos(theta), sin(theta));
};