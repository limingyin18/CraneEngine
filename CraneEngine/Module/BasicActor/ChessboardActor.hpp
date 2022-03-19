#pragma once

#include "Engine.hpp"

namespace Crane
{

    /**
     * @brief chess board
     *
     */
    class ChessboardActor : public Actor
    {
    private:
        std::shared_ptr<Crane::GraphicsPrimitive> chessboardGP;
        std::shared_ptr<CranePhysics::Cube> physicsCubeChessboard;

        CranePhysics::PositionBasedDynamics *pbd;
        Crane::Engine *ctx;
    public:
        ChessboardActor(/* args */) = default;
        ~ChessboardActor() = default;

        void init(Crane::Engine *c, CranePhysics::PositionBasedDynamics *p);
    };

}