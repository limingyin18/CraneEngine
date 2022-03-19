#include "CharacterActor.hpp"

using namespace std;
using namespace Crane;

void CharacterActor::init(Crane::Engine *c, CranePhysics::PositionBasedDynamics *p,
                          std::vector<std::shared_ptr<Crane::GraphicsPrimitive>> *b)
{
    LOGI("create human");

    ctx = c;
    pbd = p;
    bones = b;

    human = make_shared<GraphicsPrimitive>();
    human->transformer.setTransformParent(&transformer.getTransformWorld());
    primitives.push_back(human);

    {
        string name = "human";
        assets::AssetFile file;
        if (!assets::load_binaryfile((string("assets/") + "human.mesh").c_str(), file))
            throw std::runtime_error(string("Error when loading mesh "));
        assets::MeshInfo meshInfo = assets::read_mesh_info(&file);

        if (ctx->meshRepository.find(name) == ctx->meshRepository.end())
        {
            ctx->meshRepository[name] = make_shared<MeshBase>();
            auto &mesh = ctx->meshRepository[name];
            mesh->data.resize(meshInfo.vertexBuferSize / sizeof(Vertex));
            mesh->indices.resize(meshInfo.indexBuferSize / sizeof(uint32_t));
            assets::unpack_mesh(&meshInfo, file.binaryBlob.data(), file.binaryBlob.size(),
                                reinterpret_cast<char *>(mesh->data.data()),
                                reinterpret_cast<char *>(mesh->indices.data()));
            mesh.get()->setVertices([](uint32_t i, Vertex &v)
                                    { v.color = {1.f, 1.f, 1.f}; });
            mesh.get()->recomputeNormals();
        }
        human->mesh = ctx->meshRepository[name];
    }

    {
        string name = "phongBlank";
        if (ctx->materialRepository.find(name) == ctx->materialRepository.end())
            ctx->materialRepository[name] = ctx->materialBuilderPhong.build();
        human->material = ctx->materialRepository[name];
    }

    {
        Eigen::Vector3f rotationHuman{0.f, 0.f, 0.f};
        Eigen::Vector3f positionHuman{0.f, 0.f, 0.f};
        human->transformer.setRotation(rotationHuman);
        human->transformer.setPosition(positionHuman);

        bindCoffs.resize(human->mesh->data.size());
        for (uint32_t j = 0; j < human->mesh->data.size(); ++j)
        {
            float minDist = numeric_limits<float>::max();
            for (uint32_t i = 0; i < bones->size(); ++i)
            {
                Vector3f diff = human->mesh->data[j].position - (*bones)[i]->transformer.getPosition();
                float dist = (diff).norm();
                if (dist < minDist)
                {
                    minDist = dist;
                    bindCoffs[j].index = i;
                    bindCoffs[j].diff = diff;
                }
            }
        }

        for (uint32_t i = 0; i < human->mesh->data.size(); ++i)
        {
            human->mesh->data[i].position = (*bones)[bindCoffs[i].index]->transformer.getPosition() + bindCoffs[i].diff;
        }
    }
}

void CharacterActor::update()
{
    for (uint32_t i = 0; i < human->mesh->data.size(); ++i)
    {
        human->mesh->data[i].position = (*bones)[bindCoffs[i].index]->transformer.getPosition() + bindCoffs[i].diff;
    }

    for (size_t i = 0; i < human->mesh->data.size(); i++)
        ctx->vertices[human->vertBufferOffset + i] = human->mesh->data[i];
    ctx->vertBuff.update(ctx->vertices.data());
}