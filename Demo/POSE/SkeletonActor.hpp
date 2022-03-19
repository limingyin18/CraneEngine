#pragma once

#include "Engine.hpp"

/**
 * @brief skeleton actor class 
 * 
 */
class SkeletonActor : public Crane::Actor
{
public:
    std::vector<std::shared_ptr<Crane::GraphicsPrimitive>> bones;
    std::vector<uint32_t> numBoneVertices; // count of vertices each bone related to
    std::vector<assets::VertexBoneData> vertexBoneData;
    std::vector<assets::BoneVerticesData> vertexWeights;

    std::shared_ptr<Crane::GraphicsPrimitive> soldierGP;

    CranePhysics::PositionBasedDynamics *pbd;
    Crane::Engine *ctx;
public:
    void init(Crane::Engine * c, CranePhysics::PositionBasedDynamics *p);
    void update(const std::array<std::pair<std::pair<float, float>, float>, 17> &keypoints);
};