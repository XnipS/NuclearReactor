#pragma once

#include <cstdlib>
#include <string>
#include <vector>

#include "VectorMath.h"

struct ParticleStats {
    double pos_x;
    double pos_y;

    double vel_x;
    double vel_y;
};

class ReactorStatistics {
private:
    std::vector<int> m_reactivity;
    int m_max = 60;

public:
    std::vector<int> GetStats() const { return m_reactivity; }
    int GetMax() const { return m_max; }
    inline void AddData(int stat)
    {
        m_reactivity.push_back(stat);
        if (m_reactivity.size() > m_max) {
            m_reactivity.erase(m_reactivity.begin());
        }
    };
    inline void ZeroGraph()
    {
        for (int i = 0; i < m_max; i++) {
            AddData(0);
        }
    };
};

struct ReactorSettings {
    int fissionNeutronCount = 3;
    float fissionNeutronSpeed = 200;
    float decayChance = 0.001;
    float regenerateChance = 0.01;
    float xenonDecayChance = 0.001;
    float heatDissipate = 5;
    float heatTransfer = 15;
    float waterAbsorptionChance = 0.001;
    ReactorStatistics stats;

    int rodHeight_1 = 50;
    int rodHeight_2 = 50;
    int rodHeight_3 = 50;
    int rodHeight_4 = 50;
    int rodHeight_5 = 50;
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
    void LinkReactorMaterials(std::vector<CircleData>* newPos);
    void LinkNeutrons(std::vector<CircleData>* newPos);
    void LinkReactorWater(std::vector<RectangleData>* newPos);
    void LinkReactorRod(std::vector<RectangleData>* newPos);
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

    return VM::Vector2(cos(theta), sin(theta));
};