#pragma once
#include <VectorMath.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <cmath>
#include <vector>

#include "renderEngine.h"

class neutron {
public:
    VM::Vector2 position = VM::Vector2(0, 0);
    VM::Vector2 velocity = VM::Vector2(0, 0);
    int id;
    bool fast;
    neutron(double x, double y, int newid, bool fastNeutron)
    {
        position = VM::Vector2(x, y);
        id = newid;
        fast = fastNeutron;
    }
};

class water {
public:
    VM::Vector2Int position = VM::Vector2Int(0, 0);
    float temperature;
    water(int x, int y, float temp = 0)
    {
        position = VM::Vector2Int(x, y);
        temperature = temp;
    }
};

class atom {
public:
    VM::Vector2Int position = VM::Vector2Int(0, 0);
    int element = 0; // 0 = Inert, 1 = U-235
    atom(int x, int y, int e)
    {
        position = VM::Vector2Int(x, y);
        element = e;
    }
    bool canFission()
    {
        if (element == 1) {
            return true;
        } else {
            return false;
        }
    };
};

class controlRod {
public:
    float xPosition;
    float height;
    bool moderator;

    controlRod(float x, float insertion, bool mod)
    {
        xPosition = x;
        height = insertion;
        moderator = mod;
    }
};

class fluidEngine {
public:
    // Standard
    fluidEngine();
    ~fluidEngine();
    void Start(renderEngine* ren);
    void Update();
    // Reactor Alterations
    void AddReactorMaterial(int x, int y, int element);
    void AddControlRod(int x, int h, bool moderator);
    void SetControlRodHeight(int id, int h);
    void AddNeutron(int x, int y, bool fast);
    void AddWater(int x, int y);
    void DestroyNeutron(int id);
    void ClearNeutrons();
    // Engine -> Renderer Linkage
    void LinkReactorMaterialToMain(std::vector<CircleData>* newPositions);
    void LinkReactorRodToMain(std::vector<RectangleData>* newPositions);
    void LinkNeutronsToMain(std::vector<CircleData>* newPositions);
    void LinkReactorWaterToMain(std::vector<RectangleData>* newPositions);
    // Engine -> Sound Linkage
    bool isPlayingSound = false;
    // Engine -> UI Linkage
    int neutronCount;
    ReactorSettings settings;

private:
    // Neutron Updates
    void CollisionUpdate(neutron* particle);
    void ContainerUpdate(neutron* particle);
    void PositionUpdate(neutron* particle);
    // Atom (Reactor Material) Updates
    void DecayUpdate(atom* particle);
    void RegenUpdate(atom* particle);
    void RegenInert();
    // Water Updates
    void HeatTransferUpdate(water* particle);
    // UI Calculations
    float AverageReactorTemperature();
    int GetXenonCount();

    // Reactor
    std::vector<atom> reactorMaterial;
    std::vector<neutron> neutrons;
    std::vector<water> reactorWater;
    std::vector<controlRod> controlRods;
};