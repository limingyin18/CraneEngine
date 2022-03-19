#include "SoldierActor.hpp"

using namespace std;
using namespace Crane;

void SoldierActor::init(Crane::Engine *ctx, CranePhysics::PositionBasedDynamics *p)
{
    LOGI("create soldier");

    soldierGP = make_shared<GraphicsPrimitive>();
    {
        string name = "Roman-Soldier";
        assets::AssetFile file;
        if (!assets::load_binaryfile((string("assets/RomanSoldier/") + "Roman-Soldier.mesh").c_str(), file))
            throw std::runtime_error(string("Error when loading mesh "));
        assets::MeshInfo meshInfo = assets::read_mesh_info(&file);

        shared_ptr<Crane::MeshBase> mesh;
        if (ctx->meshRepository.find(name) == ctx->meshRepository.end())
        {
            ctx->meshRepository[name] = make_shared<MeshBase>();
            ctx->meshRepository[name]->data.resize(meshInfo.vertexBuferSize / sizeof(Vertex));
            ctx->meshRepository[name]->indices.resize(meshInfo.indexBuferSize / sizeof(uint32_t));
            assets::unpack_mesh(&meshInfo, file.binaryBlob.data(), file.binaryBlob.size(),
                                reinterpret_cast<char *>(ctx->meshRepository[name]->data.data()),
                                reinterpret_cast<char *>(ctx->meshRepository[name]->indices.data()));
            ctx->meshRepository[name].get()->setVertices([](uint32_t i, Vertex &v)
                                                         { v.color = {1.f, 1.f, 1.f}; });
            ctx->meshRepository[name].get()->recomputeNormals();
        }
        mesh = ctx->meshRepository[name];
        soldierGP->mesh = mesh;
    }

    {
        string name = "phongBlank";
        if (ctx->materialRepository.find(name) == ctx->materialRepository.end())
            ctx->materialRepository[name] = ctx->materialBuilderPhong.build();
        soldierGP->material = ctx->materialRepository[name];
    }

    {
        Vector3f posSoldier{0.f, 0.f, -5.0f};
        soldierGP->transformer.setPosition(posSoldier);
    }
    primitives.push_back(soldierGP);
}
