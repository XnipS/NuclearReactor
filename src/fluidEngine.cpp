#include "../include/fluidEngine.h"

#include <cmath>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <vector>

#include "../include/core.h"
#include "../include/renderEngine.h"
#include "VectorMath.h"

fluidEngine::fluidEngine() {};
fluidEngine::~fluidEngine() {};

renderEngine* renderer;
VM::Vector2 startingVelocity(0.0001, 0.0001);
bool refreshNeutrons = false;

// Spawn new reactor material
void fluidEngine::AddReactorMaterial(int x, int y, int element)
{
    atom mat = *new atom(x, y, element);
    reactorMaterial.push_back(mat);
};

// Spawn new neutron
void fluidEngine::AddNeutron(int x, int y)
{
    neutron mat = *new neutron(x, y, neutrons.size());
    mat.acceleration = *new VM::Vector2(RandomRange(-NR_NEUTRON_KE, NR_NEUTRON_KE), RandomRange(-NR_NEUTRON_KE, NR_NEUTRON_KE));
    neutrons.push_back(mat);
};

// Initialise fluid engine
void fluidEngine::Start(renderEngine* ren)
{
    printf("Fluid Engine Initialised\n");
    renderer = ren;
};

// Keep/collide in/with container
void fluidEngine::Reflect(double* input) { *input *= -(1.0 - settings.dampen); }

// Do collision check on all particles
/* void fluidEngine::CollisionUpdate(fluidParticle* particle)
{
    VM::Vector2 collisionAxis(0, 0);

    for (int j = 0; j < sand.size(); j++) {
        // if (*particle != sand[j]) {
        double dist;
        VectorDistance(&particle->position, &sand[j].position, &dist);
        const double min_dist = particle->radius + sand[j].radius;
        if (dist < min_dist) {
            if (dist == 0) {
                // printf("Zero distance\n");
                //  distance = sand[i].radius + sand[j].radius;
                continue;
            }

            VM::VectorSubtract(&collisionAxis, &sand[j].position,
                &particle->position);

            VM::VectorNormalise(&collisionAxis);
            const double delta = (particle->radius + sand[j].radius) - dist;
            VM::VectorScalarMultiply(&collisionAxis, &collisionAxis, 0.5f * delta);

            VM::VectorSum(&sand[j].position, &sand[j].position, &collisionAxis);
            VM::VectorSubtract(&particle->position, &particle->position,
                &collisionAxis);
        }
    }
    //}
} */

void fluidEngine::PositionUpdate(neutron* particle)
{
    VM::Vector2 velocity(0, 0);
    VM::VectorSubtract(&velocity, &particle->position,
        &particle->position_old); // delta_x
    particle->position_old = particle->position; // iterate x

    VM::VectorScalarMultiply(&particle->acceleration, &particle->acceleration,
        NE_DELTATIME * NE_DELTATIME); // a * t

    VM::VectorSum(&velocity, &velocity,
        &particle->acceleration); // v = at + v0

    VM::VectorSum(&particle->position, &particle->position,
        &velocity); // x = x0 + v

    // VM::VectorScalarMultiply(&particle->acceleration,
    // &particle->acceleration,
    //                          FB_DELTATIME * FB_DELTATIME);

    // VM::VectorSum(&velocity, &velocity, &particle->acceleration);

    // VM::VectorSum(&particle->position, &particle->position, &velocity);

    // particle->acceleration = VM::Vector2(0, 0);
};

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

void fluidEngine::DestroyNeutron(int id)
{
    for (int i = 0; i < neutrons.size(); i++) {
        if (neutrons[i].id == id) {
            neutrons.erase(neutrons.begin() + i);
        }
    }
    refreshNeutrons = true;
};

// Fluid engine tick
void fluidEngine::Update()
{

    // Calculate approximate total kinetic energy
    /*     float energy = 0;
        for (int t = 0; t < sand.size(); t++) {
            double magnitude;
            VectorMagnitude(&sand[t].position_old, &magnitude);
            energy += 0.5 * magnitude * magnitude;
        }
        if (renderer->currentDebugInfo.size() >= 2) {
            std::ostringstream ss;
            ss << "Current Energy: ";
            ss << std::to_string(energy);
            ss << " J";
            renderer->currentDebugInfo[1] = ss.str();

            double starting;
            VectorMagnitude(&startingVelocity, &starting);

            starting = (0.5 * starting * starting) * SandCount();

            std::ostringstream sss;
            sss << "Starting Energy: ";
            sss << std::to_string(starting);
            sss << " J";
            renderer->currentDebugInfo[0] = sss.str();

        } else {
            renderer->currentDebugInfo.push_back("INCOMING!");
            renderer->currentDebugInfo.push_back("INCOMING!");
        } */

    // Physics tick
    for (int i = 0; i < neutrons.size(); i++) {
        // GravityUpdate(&neutrons[i]);
        // Physics steps
        // for (int x = 0; x < settings.collisionCalcCount; x++) {
        //   CollisionUpdate(&neutrons[i]);
        //}
        PositionUpdate(&neutrons[i]);
        ContainerUpdate(&neutrons[i]);
    }

    // Update current sand stats
    /*     ParticleStats p;
        p.pos_x = sand[0].position.x;
        p.pos_y = sand[0].position.y;
        p.vel_x = sand[0].acceleration.x;
        p.vel_y = sand[0].acceleration.y;
        settings.particle.AddParticle(&p); */
}

// Encode reactor data to render data
void fluidEngine::LinkReactorMaterialToMain(
    std::vector<CircleData>* updatedParticles)
{
    // Render sand
    if (renderer->ClearSand()) {
        updatedParticles->clear();
    }

    for (int i = 0; i < reactorMaterial.size(); i++) {
        // Rounding
        VM::Vector2 temp = *new VM::Vector2((reactorMaterial[i].position.x * RR_SCALE) + RR_SCALE / 2, (reactorMaterial[i].position.y * RR_SCALE) + RR_SCALE / 2);
        // VM::VectorScalarMultiply(&temp, &temp, scale);
        CircleData circle(temp, (RR_SCALE / 2) - RR_PADDING, reactorMaterial[i].element);
        if (updatedParticles->size() <= i) {
            updatedParticles->push_back(circle);
        } else {
            (*updatedParticles)[i].position = temp;
            (*updatedParticles)[i].radius = (RR_SCALE / 2) - RR_PADDING;
            (*updatedParticles)[i].colourID = reactorMaterial[i].element;
        }
    }
}

// Encode neutron data to render data
void fluidEngine::LinkNeutronsToMain(
    std::vector<CircleData>* updatedParticles)
{

    // Render sand
    if (refreshNeutrons) {
        updatedParticles->clear();
        refreshNeutrons = false;
    }

    for (int i = 0; i < neutrons.size(); i++) {
        // Rounding
        VM::Vector2 temp = *new VM::Vector2((neutrons[i].position.x * RR_SCALE) + RR_SCALE / 2, (neutrons[i].position.y * RR_SCALE) + RR_SCALE / 2);
        // VM::VectorScalarMultiply(&temp, &temp, scale);
        CircleData circle(temp, (RR_SCALE / 3) - RR_PADDING, -1);
        if (updatedParticles->size() <= i) {
            updatedParticles->push_back(circle);
        } else {
            (*updatedParticles)[i].position = temp;
            (*updatedParticles)[i].radius = (RR_SCALE / 3) - RR_PADDING;
            (*updatedParticles)[i].colourID = -1;
        }
    }
}