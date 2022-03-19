#pragma once

#include "Engine.hpp"

/**
 * @brief Soldier 
 * 
 */
class SoldierActor : public Crane::Actor
{
private:
    /* data */
    std::shared_ptr<Crane::GraphicsPrimitive> soldierGP;

    CranePhysics::PositionBasedDynamics *pbd;

public:
    void init(Crane::Engine * ctx, CranePhysics::PositionBasedDynamics *p);
    void update();
};