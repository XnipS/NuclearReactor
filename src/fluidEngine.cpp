#include "../include/fluidEngine.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>

#include "../include/core.h"
#include "../include/renderEngine.h"
#include "VectorMath.h"
#include "imgui.h"

fluidEngine::fluidEngine() {};
fluidEngine::~fluidEngine() {};

renderEngine* renderer;
VM::Vector2 startingVelocity(0.0001, 0.0001);
bool refreshNeutrons = false;
int neutronCurrentID = 0;
int statUpdate = NE_TARGET_TICKRATE;

// Spawn new reactor material
void fluidEngine::AddReactorMaterial(int x, int y, int element)
{
    atom mat = atom(x, y, element);
    reactorMaterial.push_back(mat);
};

// Spawn new water
void fluidEngine::AddWater(int x, int y)
{
    water mat = water(x, y);
    reactorWater.push_back(mat);
};

// Spawn new neutron
void fluidEngine::AddNeutron(int x, int y, bool fast)
{
    neutron mat = neutron(x, y, neutronCurrentID, fast);
    neutronCurrentID++;
    VM::Vector2 acc = RandomUnitVector();
    float speed = settings.fissionNeutronSpeed;
    if (fast) {
        speed = settings.fissionFastNeutronSpeed;
    }
    mat.velocity = VM::Vector2(acc.x * speed, acc.y * speed);
    neutrons.push_back(mat);
};

// Spawn new control rod
void fluidEngine::AddControlRod(int x, int h, bool moderator)
{
    controlRod mat = controlRod(x, h, moderator);
    controlRods.push_back(mat);
};

// Initialise fluid engine
void fluidEngine::Start(renderEngine* ren)
{
    printf("Fluid Engine Initialised\n");
    renderer = ren;
};

// Count xenon atoms in reactor
int fluidEngine::GetXenonCount()
{
    int sum = 0;
    for (int i = 0; i < reactorMaterial.size(); i++) {
        if (reactorMaterial[i].element == 2) {
            sum++;
        }
    }
    return sum;
}

// Do collision check on all particles
void fluidEngine::CollisionUpdate(neutron* particle)
{
    // Check for reactor material collisions
    for (int j = 0; j < reactorMaterial.size(); j++) {
        double dist;
        VectorDistanceInt(&reactorMaterial[j].position, &particle->position, &dist);
        const double min_dist = 0.5;
        if (dist < min_dist) {
            if (!particle->fast) {
                // Thermal Neutrons only collide with reactor material
                if (reactorMaterial[j].element == 1) {
                    // Is U-235 -> Can Fission!
                    reactorMaterial[j].element = 0;
                    DestroyNeutron(particle->id);
                    RegenInert();
                    for (int i = 0; i < settings.fissionNeutronCount; i++) {
                        AddNeutron(reactorMaterial[j].position.x, reactorMaterial[j].position.y, true);
                    }
                    isPlayingSound = true;
                    break;
                } else if (reactorMaterial[j].element == 2) {
                    // Is Xe-135 -> Can Stabilise!
                    reactorMaterial[j].element = 0;
                    DestroyNeutron(particle->id);
                    break;
                } else {
                    continue;
                }
            }
        }
    }
    // Check for control rod collisions
    // Absorb both fast and slow neutrons
    for (int j = 0; j < controlRods.size(); j++) {
        // Check x
        if (particle->position.x + 0.5 > (controlRods[j].xPosition - 0.5) && particle->position.x + 0.5 < (controlRods[j].xPosition + 0.5)) {
            // Check y
            if (particle->position.y + 0.5 < ((controlRods[j].height / 100) * NR_SIZE_Y)) {
                // In bounds

                // Standard Control Rod
                DestroyNeutron(particle->id);
                break;

                /*  if (!controlRods[j].moderator) {
                     // Standard Control Rod
                     DestroyNeutron(particle->id);
                     break;
                 } else {
                     // Moderator
                     if (particle->fast) {

                         particle->velocity.x = -particle->velocity.x; // Reflect off moderator
                         VM::VectorNormalise(&particle->velocity); // Normalise velocity vector
                         VM::VectorScalarMultiply(&particle->velocity, &particle->velocity, settings.fissionNeutronSpeed); // Scalar multiply by new speed
                         particle->fast = false; // No longer fast neutron
                     }
                 } */
            } else if (particle->position.y + 0.5 > (((controlRods[j].height / 100)) * NR_SIZE_Y) + RR_CR_PADDING) {
                // Moderator
                if (particle->fast) {

                    particle->velocity.x = -particle->velocity.x; // Reflect off moderator
                    VM::VectorNormalise(&particle->velocity); // Normalise velocity vector
                    VM::VectorScalarMultiply(&particle->velocity, &particle->velocity, settings.fissionNeutronSpeed); // Scalar multiply by new speed
                    particle->fast = false; // No longer fast neutron
                }
            }
        }
    }
}

// Apply physics to neutrons
void fluidEngine::PositionUpdate(neutron* particle)
{
    VM::Vector2 temp(0, 0);

    VM::VectorScalarMultiply(&temp, &particle->velocity,
        NE_DELTATIME);

    VM::VectorSum(&particle->position, &particle->position,
        &temp);
};

// Inert atoms radiate random neutrons
void fluidEngine::DecayUpdate(atom* particle)
{
    if (particle->element == 0) {
        // Inert -> Release radiation
        if (RandomRange(0.0, 1.0) < settings.decayChance * NE_DELTATIME) {
            AddNeutron(particle->position.x, particle->position.y, true);
        }
        // Inert -> Can decay to Xenon
        if (RandomRange(0.0, 1.0) < settings.xenonDecayChance * NE_DELTATIME) {
            particle->element = 2;
        }
    }
};

// Regen random inert atom
void fluidEngine::RegenInert()
{
    bool regenerated = false;
    while (!regenerated) {
        int test = RandomRangeInt(0, reactorMaterial.size());
        if (reactorMaterial[test].element == 0) {
            reactorMaterial[test].element = 1;
            regenerated = true;
        }
    }
};

// Heat water if neutron is touching
void fluidEngine::HeatTransferUpdate(water* particle)
{
    if (particle->temperature > 0) {
        particle->temperature -= settings.heatDissipate * NE_DELTATIME;
    } else {
        particle->temperature = 0;
    }
    for (int j = 0; j < neutrons.size(); j++) {
        double dist;
        VectorDistanceInt(&particle->position, &neutrons[j].position, &dist);

        if (dist < NR_WATER_RANGE) {
            particle->temperature += settings.heatTransfer * NE_DELTATIME;
            if (particle->temperature < 100) {
                if (RandomRange(0.0, 1.0) < settings.waterAbsorptionChance * NE_DELTATIME) {
                    DestroyNeutron(neutrons[j].id);
                }
            }
        } else {
            continue;
        }
    }
    if (particle->position.y == NR_SIZE_Y - 1) {
        particle->temperature = NR_WATER_TEMP_OFFSET;
    }
    if (particle->position.y != 0) {
        int val = (particle->position.y - 1) + ((particle->position.x) * NR_SIZE_Y); // Id of water packet above
        float delta = (particle->temperature - reactorWater[val].temperature) * NE_DELTATIME * settings.waterFlow;
        reactorWater[val].temperature += delta;
        particle->temperature -= delta;
    }
};

// Remove neutrons out of containment
void fluidEngine::ContainerUpdate(neutron* particle)
{
    bool escaped = false;
    float alter = 0.5;
    if (particle->position.x + alter < 0) {
        escaped = true;
    }
    if (particle->position.x + alter > NR_SIZE_X) {
        escaped = true;
    }
    if (particle->position.y + alter < 0) {
        escaped = true;
    }
    if (particle->position.y + alter > NR_SIZE_Y) {
        escaped = true;
    }

    if (escaped) {
        DestroyNeutron(particle->id);
    }
};

// Clear all neutrons
void fluidEngine::ClearNeutrons()
{
    neutrons.clear();
    refreshNeutrons = true;
};

// Destroy specific neutron
void fluidEngine::DestroyNeutron(int id)
{
    for (int i = 0; i < neutrons.size(); i++) {
        if (neutrons[i].id == id) {
            neutrons.erase(neutrons.begin() + i);
            break;
        }
    }
    refreshNeutrons = true;
};

// Set control rod height
void fluidEngine::SetControlRodHeight(int id, int h)
{
    controlRods[id].height = h;
};

// Calculate average water temperature in reactor
float fluidEngine::AverageReactorTemperature()
{
    float avg = 0;
    for (int i = 0; i < reactorWater.size(); i++) {

        avg += (reactorWater[i].temperature + NR_WATER_TEMP_OFFSET);
    }

    return (avg / reactorWater.size());
}

// Fluid engine tick
void fluidEngine::Update()
{
    // Physics tick
    for (int i = 0; i < neutrons.size(); i++) {
        CollisionUpdate(&neutrons[i]);
        ContainerUpdate(&neutrons[i]);
        PositionUpdate(&neutrons[i]);
    }
    for (int i = 0; i < reactorMaterial.size(); i++) {
        DecayUpdate(&reactorMaterial[i]);
    }
    for (int i = 0; i < reactorWater.size(); i++) {
        HeatTransferUpdate(&reactorWater[i]);
    }

    // Update current statistics
    if (statUpdate <= 0) {

        settings.stats.AddXenonData(GetXenonCount());
        settings.stats.AddReactionData(neutrons.size());
        settings.stats.AddTempData(AverageReactorTemperature());
        statUpdate = NE_TARGET_TICKRATE;
    } else {
        statUpdate--;
    }
}

// Encode reactor data to render data
void fluidEngine::LinkReactorMaterialToMain(
    std::vector<CircleData>* updatedParticles)
{
    for (int i = 0; i < reactorMaterial.size(); i++) {
        // Rounding
        VM::Vector2 temp((reactorMaterial[i].position.x * RR_SCALE) + RR_SCALE / 2, (reactorMaterial[i].position.y * RR_SCALE) + RR_SCALE / 2);
        // VM::VectorScalarMultiply(&temp, &temp, scale);
        CircleData circle(temp, (RR_SCALE / 2) - RR_ATOM_PADDING, reactorMaterial[i].element);
        if (updatedParticles->size() <= i) {
            updatedParticles->push_back(circle);
        } else {
            (*updatedParticles)[i].position = temp;
            (*updatedParticles)[i].radius = (RR_SCALE / 2) - RR_ATOM_PADDING;
            (*updatedParticles)[i].colourID = reactorMaterial[i].element;
        }
    }
}

// Encode neutron data to render data
void fluidEngine::LinkNeutronsToMain(
    std::vector<CircleData>* updatedParticles)
{
    // Clear neutrons
    if (refreshNeutrons) {
        updatedParticles->clear();
        refreshNeutrons = false;
    }

    for (int i = 0; i < neutrons.size(); i++) {
        // Rounding
        VM::Vector2 temp((neutrons[i].position.x * RR_SCALE) + RR_SCALE / 2, (neutrons[i].position.y * RR_SCALE) + RR_SCALE / 2);
        int colorId = 0;
        if (neutrons[i].fast) {
            colorId = 1;
        }
        CircleData circle(temp, (RR_SCALE / 5), colorId);
        if (updatedParticles->size() <= i) {
            updatedParticles->push_back(circle);
        } else {
            (*updatedParticles)[i].position = temp;
            (*updatedParticles)[i].radius = (RR_SCALE / 5);
            (*updatedParticles)[i].colourID = colorId;
        }
    }
}

// Encode water data to render data
void fluidEngine::LinkReactorWaterToMain(
    std::vector<RectangleData>* updatedParticles)
{
    for (int i = 0; i < reactorWater.size(); i++) {
        // Rounding
        VM::Vector2 temp((reactorWater[i].position.x * RR_SCALE) + RR_SCALE / 2, (reactorWater[i].position.y * RR_SCALE) + RR_SCALE / 2);
        VM::Vector2 size((RR_SCALE / 2) - RR_WATER_PADDING, (RR_SCALE / 2) - RR_WATER_PADDING);
        int colour = -1;
        if (reactorWater[i].temperature < 0) {
            // Blue
            colour = 0;
        } else if (reactorWater[i].temperature > 100) {
            colour = -1;
        } else {
            colour = reactorWater[i].temperature * 2.55;
        }

        RectangleData rect(temp, size, colour);

        if (updatedParticles->size() <= i) {
            updatedParticles->push_back(rect);
        } else {
            (*updatedParticles)[i].position = temp;
            (*updatedParticles)[i].size = size;
            (*updatedParticles)[i].colourID = colour;
        }
    }
}

// Encode water data to render data
void fluidEngine::LinkReactorRodToMain(
    std::vector<RectangleData>* updatedParticles)
{
    for (int i = 0; i < controlRods.size(); i++) {
        // Control Rod
        VM::Vector2 pos1((controlRods[i].xPosition * RR_SCALE), (((controlRods[i].height / 100) * RR_SCALE * NR_SIZE_Y) / 2));
        VM::Vector2 size1((RR_SCALE / 3), ((controlRods[i].height / 100) * RR_SCALE * NR_SIZE_Y));
        // Moderator
        VM::Vector2 pos2((controlRods[i].xPosition * RR_SCALE), ((((controlRods[i].height / 100) * NR_SIZE_Y + NR_SIZE_Y + RR_CR_PADDING) * RR_SCALE) / 2));
        VM::Vector2 size2((RR_SCALE / 3), (((1 - controlRods[i].height / 100) * NR_SIZE_Y - RR_CR_PADDING) * RR_SCALE));
        // Connecting Rod
        VM::Vector2 pos3(((controlRods[i].xPosition) * RR_SCALE), (NR_SIZE_Y * RR_SCALE) / 2);
        VM::Vector2 size3((RR_SCALE / 6), NR_SIZE_Y * RR_SCALE);

        RectangleData rect1(pos1, size1, 0);
        RectangleData rect2(pos2, size2, 1);
        RectangleData rect3(pos3, size3, -1);

        if (updatedParticles->size() <= i * 3) {
            updatedParticles->push_back(rect1);
            updatedParticles->push_back(rect2);
            updatedParticles->push_back(rect3);
        } else {
            (*updatedParticles)[i * 3].position = pos3;
            (*updatedParticles)[i * 3].size = size3;
            (*updatedParticles)[i * 3].colourID = -1;

            (*updatedParticles)[i * 3 + 1].position = pos2;
            (*updatedParticles)[i * 3 + 1].size = size2;
            (*updatedParticles)[i * 3 + 1].colourID = 1;

            (*updatedParticles)[i * 3 + 2].position = pos1;
            (*updatedParticles)[i * 3 + 2].size = size1;
            (*updatedParticles)[i * 3 + 2].colourID = 0;
        }
    }
}