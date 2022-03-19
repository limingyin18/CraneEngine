#include "ChessboardActor.hpp"

using namespace std;
using namespace Crane;

void ChessboardActor::init(Crane::Engine *c, CranePhysics::PositionBasedDynamics *p)
{
    LOGI("create chessboard");

    ctx = c;
    pbd = p;

    chessboardGP = make_shared<GraphicsPrimitive>();
	{
		string name = "chessboard";
        if (ctx->meshRepository.find(name) == ctx->meshRepository.end())
			ctx->meshRepository[name] = make_shared<Crane::Chessboard>(11, 11);
        shared_ptr<Crane::Chessboard> mesh = dynamic_pointer_cast<Crane::Chessboard>(ctx->meshRepository[name]);
		mesh->setVertices([](uint32_t, Vertex& v) {v.position *= 100; });
		chessboardGP->mesh = mesh;
	}
	{
		string name = "phongBlank";
        if (ctx->materialRepository.find(name) == ctx->materialRepository.end())
            ctx->materialRepository[name] = ctx->materialBuilderPhong.build();
        chessboardGP->material = ctx->materialRepository[name];
	}
	{
        Eigen::Vector3f pos{0.f, 0.f, 0.f};
        chessboardGP->transformer.setPosition(pos);
	}
	chessboardGP->transformer.setTransformParent(&transformer.getTransformWorld());
	primitives.push_back(chessboardGP);

	// physics
	physicsCubeChessboard = std::make_shared<CranePhysics::Cube>();
	physicsCubeChessboard->invMass = 0.f;
	Vector3f pos = primitives[0]->transformer.getPosition();
	Matrix4f trans = *(primitives[0]->transformer.getTransformParent());
	Vector4f posT =  trans* Vector4f{pos[0], pos[1], pos[2], 1.0f};

	physicsCubeChessboard->position = Vector3f{posT[0], posT[1], posT[2]};
	physicsCubeChessboard->width = 100.f;
	physicsCubeChessboard->depth = 100.f;
	physicsCubeChessboard->height = 0.1f;
	pbd->rigidbodies.push_back(physicsCubeChessboard);
}