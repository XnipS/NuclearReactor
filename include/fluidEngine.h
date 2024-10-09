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
    VM::Vector2 position = *new VM::Vector2(0, 0);
    VM::Vector2 position_old = *new VM::Vector2(0, 0);
    VM::Vector2 acceleration = *new VM::Vector2(0, 0);
    int id;
    neutron(double x, double y, int newid)
    {
        position = *new VM::Vector2(x, y);
        position_old = *new VM::Vector2(x, y);
        id = newid;
    }
};

class water {
public:
    VM::Vector2Int position = *new VM::Vector2Int(0, 0);
    float temperature;
    water(int x, int y, float temp = 0)
    {
        position = *new VM::Vector2Int(x, y);
        temperature = temp;
    }
};

class atom {
public:
    VM::Vector2Int position = *new VM::Vector2Int(0, 0);
    int element = 0; // 0 = Inert, 1 = U-235
    atom(int x, int y, int e)
    {
        position = *new VM::Vector2Int(x, y);
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

class fluidEngine {
public:
    fluidEngine();
    ~fluidEngine();
    void Start(renderEngine* ren);
    void Update();
    void AddReactorMaterial(int x, int y, int element);
    void LinkReactorMaterialToMain(std::vector<CircleData>* newPositions);
    void AddNeutron(int x, int y);
    void AddWater(int x, int y);
    void DestroyNeutron(int id);
    void ClearNeutrons();
    void LinkNeutronsToMain(std::vector<CircleData>* newPositions);
    void LinkReactorWaterToMain(std::vector<RectangleData>* newPositions);
    // int SandCount() { return sand.size(); }
    ReactorSettings settings;

private:
    // Neutron Updates
    void CollisionUpdate(neutron* particle);
    void ContainerUpdate(neutron* particle);
    void PositionUpdate(neutron* particle);
    // Atom (Reactor Material) Updates
    void DecayUpdate(atom* particle);
    void RegenUpdate(atom* particle);
    // Water Updates
    void HeatTransferUpdate(water* particle);

    void Reflect(double* input);
    std::vector<atom> reactorMaterial;
    std::vector<neutron> neutrons;
    std::vector<water> reactorWater;
};