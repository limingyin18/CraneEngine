#include "ClothActor.hpp"

using namespace std;
using namespace Crane;

void ClothActor::init(Crane::Engine *ctx)
{
    LOGI("create cloth")

    Eigen::Vector3f posCloth{0.f, 0.f, -1.f};
    transformer.setPosition(posCloth);

    GraphicsPrimitive sphereGP;
    {
        string name = "sphere20";
        uint32_t nSphere = 20;
        float radius = 1.f;
        if (ctx->meshRepository.find(name) == ctx->meshRepository.end())
            ctx->meshRepository[name] = make_shared<Crane::Sphere>(nSphere, nSphere, radius);
        shared_ptr<Crane::Sphere> mesh = dynamic_pointer_cast<Crane::Sphere>(ctx->meshRepository[name]);
        mesh->setVertices([](uint32_t i, Vertex &v)
                          { v.color = {1.0f, 0.f, 0.f}; });
        sphereGP.mesh = mesh;
    }
    {
        string name = "phongBlank";
        if (ctx->materialRepository.find(name) == ctx->materialRepository.end())
            ctx->materialRepository[name] = ctx->materialBuilderPhong.build();
        sphereGP.material = ctx->materialRepository[name];
    }
    {
        Eigen::Vector3f posSphere{0.f, 0.f, -1.f};
        sphereGP.transformer.setPosition(posSphere);
    }
    primitives.push_back(sphereGP);
    
    //ctx->renderables.emplace_back(primitives[0].mesh.get(), primitives[0].material.get(), &primitives[0].transformer.getTransform());
}