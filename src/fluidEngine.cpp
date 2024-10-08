#include "../include/fluidEngine.h"

#include <cmath>
#include <cstdio>
#include <sstream>

#include "../include/core.h"
#include "../include/renderEngine.h"
#include "VectorMath.h"

fluidEngine::fluidEngine() {};
fluidEngine::~fluidEngine() {};

renderEngine* renderer;
VM::Vector2 startingVelocity(0.0001, 0.0001);

// Spawn new reactor material
void fluidEngine::AddReactorMaterial(int x, int y, int element)
{
    atom mat = *new atom(x, y, element);
    reactorMaterial.push_back(mat);
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

void fluidEngine::PositionUpdate(fluidParticle* particle)
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
    /*     for (int i = 0; i < sand.size(); i++) {
            GravityUpdate(&sand[i]);
            ContainerUpdate(&sand[i]);
            // Physics steps
            for (int x = 0; x < settings.collisionCalcCount; x++) {
                CollisionUpdate(&sand[i]);
            }
            PositionUpdate(&sand[i]);
        } */

    // Update current sand stats
    /*     ParticleStats p;
        p.pos_x = sand[0].position.x;
        p.pos_y = sand[0].position.y;
        p.vel_x = sand[0].acceleration.x;
        p.vel_y = sand[0].acceleration.y;
        settings.particle.AddParticle(&p); */
}

// Encode sand data to colour data
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
        CircleData circle(temp, (1 * RR_SCALE / 2) - RR_PADDING, reactorMaterial[i].element);
        if (updatedParticles->size() <= i) {
            updatedParticles->push_back(circle);
        } else {
            (*updatedParticles)[i].position = temp;
            (*updatedParticles)[i].radius = (1 * RR_SCALE / 2) - RR_PADDING;
            (*updatedParticles)[i].colourID = reactorMaterial[i].element;
        }
    }
}