#pragma once

#include "Engine.hpp"

/**
 * @brief bone coefficient 
 * 
 */
struct BindCoff
{
	uint32_t index;
	Eigen::Vector3f diff;
};

/**
 * @brief Character actor class 
 * 
 */
class CharacterActor : public Crane::Actor
{
private:
    std::shared_ptr<Crane::GraphicsPrimitive> human;

    std::vector<std::shared_ptr<Crane::GraphicsPrimitive>> *bones;
	std::vector<BindCoff> bindCoffs;

    CranePhysics::PositionBasedDynamics *pbd;
    Crane::Engine *ctx;
public:
    void init(Crane::Engine * c, CranePhysics::PositionBasedDynamics *p,
    std::vector<std::shared_ptr<Crane::GraphicsPrimitive>> *b);
    void update();
};