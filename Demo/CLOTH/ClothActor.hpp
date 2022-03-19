#pragma once

#include "Engine.hpp"

/**
 * @brief Cloth actor class 
 * 
 */
class ClothActor : public Crane::Actor
{
private:
    /* data */
    std::shared_ptr<Crane::GraphicsPrimitive> sphereGP;
    std::shared_ptr<CranePhysics::Sphere> spherePhy;

    std::shared_ptr<Crane::GraphicsPrimitive> sphereAGP;
    std::shared_ptr<CranePhysics::Sphere> sphereAPhy;

    std::shared_ptr<Crane::GraphicsPrimitive> sphereTestGP;
    std::shared_ptr<CranePhysics::Sphere> sphereTestPhy;

    uint32_t numX, numY;
    std::shared_ptr<Crane::GraphicsPrimitive> clothGP;
    std::vector<std::shared_ptr<CranePhysics::Sphere>> clothPhys;

    CranePhysics::PositionBasedDynamics *pbd;
    Crane::Engine *ctx;

public:
    void init(Crane::Engine * c, CranePhysics::PositionBasedDynamics *p);
    void update();
    ClothActor(/* args */) = default;
    ~ClothActor() = default;
};