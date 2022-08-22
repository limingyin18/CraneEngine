#pragma once

#include "Engine.hpp"

/**
 * @brief Skybox actor class
 *
 */
class SkyboxActor : public Crane::Actor
{
private:
    /* data */
    std::shared_ptr<Crane::GraphicsPrimitive> cubeGP;
    Crane::Engine* ctx;

public:
    void init(Crane::Engine* c, CranePhysics::PositionBasedDynamics* p);
    void update() {};
    SkyboxActor(/* args */) = default;
    ~SkyboxActor() = default;
};