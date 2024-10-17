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
    // Stats to follow
    std::vector<int> m_reactivity;
    std::vector<int> m_xenon;
    std::vector<float> m_temp;
    // Stat history
    int m_max = 60;

public:
    // Pull data from memory
    std::vector<int> GetReactivityStats() const { return m_reactivity; }
    std::vector<int> GetXenonStats() const { return m_xenon; }
    std::vector<float> GetTempStats() const { return m_temp; }

    // Get stat history
    int GetMax() const { return m_max; }

    // Add reactivity data
    inline void AddReactionData(int stat)
    {
        m_reactivity.push_back(stat);
        if (m_reactivity.size() > m_max) {
            m_reactivity.erase(m_reactivity.begin());
        }
    };

    // Add xenon count data
    inline void AddXenonData(int stat)
    {
        m_xenon.push_back(stat);
        if (m_xenon.size() > m_max) {
            m_xenon.erase(m_xenon.begin());
        }
    };

    // Add average temperature data
    inline void AddTempData(float stat)
    {
        m_temp.push_back(stat);
        if (m_temp.size() > m_max) {
            m_temp.erase(m_temp.begin());
        }
    };

    // Zero all data
    inline void ZeroGraph()
    {
        for (int i = 0; i < m_max; i++) {
            AddReactionData(0);
            AddXenonData(0);
            AddTempData(0);
        }
    };
};

struct ReactorSettings {
    // Neutron settings
    int fissionNeutronCount = 3;
    float fissionNeutronSpeed = 3;
    float fissionFastNeutronSpeed = 6;
    float decayChance = 0.01;
    float waterAbsorptionChance = 0.02;
    // Reactor material settings
    float xenonDecayChance = 0.003;
    // Heat transfer settings
    float heatDissipate = 10;
    float waterFlow = 30;
    float heatTransfer = 15;
    // Graph data
    ReactorStatistics stats;
    // Rods
    float rodHeight_1 = 100;
    float rodHeight_2 = 100;
    float rodHeight_3 = 100;
    float rodHeight_4 = 100;
    float rodHeight_5 = 100;
};

// Draw circle
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

// Draw rectangle
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
    void Update();
    void Render();
    void Clean();
    bool Running() { return isRunning; };

    // Pass data to render
    void LinkSettings(ReactorSettings* set) { settings = set; };
    void LinkReactorMaterials(std::vector<CircleData>* newPos);
    void LinkNeutrons(std::vector<CircleData>* newPos);
    void LinkReactorWater(std::vector<RectangleData>* newPos);
    void LinkReactorRod(std::vector<RectangleData>* newPos);

    // User feedback
    int AddNetron() { return addNeutrons; };
    bool ClearNeutrons() { return clearAllNeutrons; };

    std::vector<std::string> currentDebugInfo; // TODO
private:
    ReactorSettings* settings;
    int tick = 0;
    bool isRunning;
    int addNeutrons = 0;
    bool clearAllNeutrons = false;
};

// Random double in range
inline double RandomRange(double fMin, double fMax)
{
    double f = (double)rand() / RAND_MAX;
    return fMin + f * (fMax - fMin);
};

// Random int in range
inline int RandomRangeInt(int fMin, int fMax)
{
    return fMin + rand() % ((fMax + 1) - fMin);
};

// Random 2D direction with magnitude of 1
inline VM::Vector2 RandomUnitVector()
{
    double theta = RandomRange(0, 2 * M_PI);

    return VM::Vector2(cos(theta), sin(theta));
};