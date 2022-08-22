#include "ClothActor.hpp"

using namespace std;
using namespace Crane;

void ClothActor::init(Crane::Engine *c, CranePhysics::PositionBasedDynamics *p)
{
    ctx = c;
    pbd = p;

    LOGI("create cloth")

    Eigen::Vector3f posCloth{0.f, 0.f, 0.f};
    transformer.setPosition(posCloth);

    sphereTestGP = make_shared<GraphicsPrimitive>();
    {
        string name = "sphere20";
        uint32_t nSphere = 20;
        float radius = 1.f;
        if (ctx->meshRepository.find(name) == ctx->meshRepository.end())
            ctx->meshRepository[name] = make_shared<Crane::Sphere>(nSphere, nSphere, radius);
        shared_ptr<Crane::Sphere> mesh = dynamic_pointer_cast<Crane::Sphere>(ctx->meshRepository[name]);
        mesh->setVertices([](uint32_t i, Vertex &v)
                          { v.color = {1.0f, 0.f, 0.f}; });
        sphereTestGP->mesh = mesh;
    }
    {
        string name = "phongBlank";
        if (ctx->materialRepository.find(name) == ctx->materialRepository.end())
            ctx->materialRepository[name] = ctx->materialBuilderPhong.build();
        sphereTestGP->material = ctx->materialRepository[name];
    }
    {
        Eigen::Vector3f posSphere{0.f, 20.f, 0.f};
        sphereTestGP->transformer.setPosition(posSphere);
    }
    sphereTestGP->transformer.setTransformParent(&transformer.getTransformWorld());
    primitives.push_back(sphereTestGP);

    {
        // physics
        sphereTestPhy = std::make_shared<CranePhysics::Sphere>();
        sphereTestPhy->invMass = 1.f;
        Vector3f pos = primitives[0]->transformer.getPosition();
        Matrix4f trans = *(primitives[0]->transformer.getTransformParent());
        Vector4f posT = trans * Vector4f{pos[0], pos[1], pos[2], 1.0f};

        sphereTestPhy->position = Vector3f{posT[0], posT[1], posT[2]};
        sphereTestPhy->positionPrime = sphereTestPhy->position;
        sphereTestPhy->radius = 1.f;
        pbd->rigidbodies.push_back(sphereTestPhy);
    }

    //
    sphereGP = make_shared<GraphicsPrimitive>();
    {
        string name = "sphere20";
        uint32_t nSphere = 20;
        float radius = 1.f;
        if (ctx->meshRepository.find(name) == ctx->meshRepository.end())
            ctx->meshRepository[name] = make_shared<Crane::Sphere>(nSphere, nSphere, radius);
        shared_ptr<Crane::Sphere> mesh = dynamic_pointer_cast<Crane::Sphere>(ctx->meshRepository[name]);
        mesh->setVertices([](uint32_t i, Vertex &v)
                          { v.color = {1.0f, 0.f, 0.f}; });
        sphereGP->mesh = mesh;
    }
    {
        string name = "phongBlank";
        if (ctx->materialRepository.find(name) == ctx->materialRepository.end())
            ctx->materialRepository[name] = ctx->materialBuilderPhong.build();
        sphereGP->material = ctx->materialRepository[name];
    }
    {
        Eigen::Vector3f posSphere{0.f, 5.f, 0.f};
        sphereGP->transformer.setPosition(posSphere);
    }
    sphereGP->transformer.setTransformParent(&transformer.getTransformWorld());
    primitives.push_back(sphereGP);

    {
        // physics
        spherePhy = std::make_shared<CranePhysics::Sphere>();
        spherePhy->invMass = 1.f;
        Vector3f pos = primitives[0]->transformer.getPosition();
        Matrix4f trans = *(primitives[0]->transformer.getTransformParent());
        Vector4f posT = trans * Vector4f{pos[0], pos[1], pos[2], 1.0f};

        spherePhy->position = Vector3f{posT[0], posT[1], posT[2]};
        spherePhy->positionPrime = spherePhy->position;
        spherePhy->radius = 1.f;
        pbd->rigidbodies.push_back(spherePhy);
    }

    sphereAGP = make_shared<GraphicsPrimitive>();
    {
        string name = "sphere20";
        uint32_t nSphere = 20;
        float radius = 1.f;
        if (ctx->meshRepository.find(name) == ctx->meshRepository.end())
            ctx->meshRepository[name] = make_shared<Crane::Sphere>(nSphere, nSphere, radius);
        shared_ptr<Crane::Sphere> mesh = dynamic_pointer_cast<Crane::Sphere>(ctx->meshRepository[name]);
        mesh->setVertices([](uint32_t i, Vertex &v)
                          { v.color = {1.0f, 0.f, 0.f}; });
        sphereAGP->mesh = mesh;
    }
    {
        string name = "phongBlank";
        if (ctx->materialRepository.find(name) == ctx->materialRepository.end())
            ctx->materialRepository[name] = ctx->materialBuilderPhong.build();
        sphereAGP->material = ctx->materialRepository[name];
    }
    {
        Eigen::Vector3f posSphere{0.f, 5.f, 2.1f};
        sphereAGP->transformer.setPosition(posSphere);
    }
    sphereAGP->transformer.setTransformParent(&transformer.getTransformWorld());
    primitives.push_back(sphereAGP);

    {
        // physics
        sphereAPhy = std::make_shared<CranePhysics::Sphere>();
        sphereAPhy->invMass = 1.f;
        Vector3f pos = sphereAGP->transformer.getPosition();
        Matrix4f trans = *(sphereAGP->transformer.getTransformParent());
        Vector4f posT = trans * Vector4f{pos[0], pos[1], pos[2], 1.0f};

        sphereAPhy->position = Vector3f{posT[0], posT[1], posT[2]};
        sphereAPhy->positionPrime = sphereAPhy->position;
        sphereAPhy->radius = 1.f;
        pbd->rigidbodies.push_back(sphereAPhy);
    }

    float dist = (sphereAPhy->position - spherePhy->position).norm();
    auto stretchingConstraint = std::make_shared<CranePhysics::Stretching>(*sphereAPhy, *spherePhy, dist, 1.f, 1.f);
    pbd->constraints.push_back(stretchingConstraint);

    // cloth
    numX = 3;
    numY = 3;
    clothGP = make_shared<GraphicsPrimitive>();
    {
        string name = "flagCloth";
        if (ctx->meshRepository.find(name) == ctx->meshRepository.end())
            ctx->meshRepository[name] = make_shared<Crane::Plane>(numX, numY);
        shared_ptr<Crane::MeshBase> mesh = dynamic_pointer_cast<Crane::MeshBase>(ctx->meshRepository[name]);
        clothGP->mesh = mesh;
    }
    {
        string nameImage = "BannerHolyRoman";
        {
            string imageName = "StandardCubeMap.tx";
            assets::AssetFile textureFile;
            if (!load_binaryfile((string("assets/") + imageName).c_str(), textureFile))
                throw runtime_error("load asset failed");

            assets::TextureInfo textureInfo = assets::read_texture_info(&textureFile);
            VkDeviceSize imageSize = textureInfo.textureSize;
            vector<uint8_t> pixels(imageSize);
            assets::unpack_texture(&textureInfo, textureFile.binaryBlob.data(), textureFile.binaryBlob.size(), (char *)pixels.data());

            int texWidth, texHeight, texChannels;
            texChannels = 4;
            texWidth = textureInfo.pages[0].width;
            texHeight = textureInfo.pages[0].height;

            tie(ctx->loadImages[nameImage], ctx->loadImageViews[nameImage]) = ctx->createTextureImage(texWidth, texHeight, texChannels, pixels.data());
            ctx->descriptorImageInfos[nameImage].imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
            ctx->descriptorImageInfos[nameImage].imageView = ctx->loadImageViews[nameImage].get();
            ctx->descriptorImageInfos[nameImage].sampler = ctx->textureSampler.get();
        }
        string name = "phongBanner";
        if (ctx->materialRepository.find(name) == ctx->materialRepository.end())
        {
            ctx->materialBuilderPhong.descriptorInfos[1][0].second = &ctx->descriptorImageInfos[nameImage];
            ctx->materialRepository[name] = ctx->materialBuilderPhong.build();
            ctx->materialBuilderPhong.descriptorInfos[1][0].second = &ctx->descriptorImageInfoBlank;
        }
        clothGP->material = ctx->materialRepository[name];
    }
    {
        Eigen::Vector3f pos{0.f, 5.f, 5.f};
        clothGP->transformer.setPosition(pos);
    }
    clothGP->transformer.setTransformParent(&transformer.getTransformWorld());
    primitives.push_back(clothGP);

    // physics
    for (uint32_t i = 0; i < clothGP->mesh->data.size(); ++i)
    {
        auto particle = std::make_shared<CranePhysics::Particle>();
        clothPhys.push_back(particle);

        Vector3f pos = clothGP->mesh->data[i].position;
        Matrix4f trans = (clothGP->transformer.getTransformWorld());
        Vector4f posT = trans * Vector4f{pos[0], pos[1], pos[2], 1.0f};
        particle->position = Vector3f{posT[0], posT[1], posT[2]};
        particle->positionPrime = particle->position;

        particle->radius = 0.1f - 0.01f;
        particle->invMass = 1.f;

        pbd->rigidbodies.push_back(particle);
    }

    // physics constraint
    float compress = 1.0f, stretch = 1.0f;
    for (uint32_t i = 0; i < clothGP->mesh->indices.size(); i = i + 3)
    {
        size_t indexA = clothGP->mesh->indices[i];
        size_t indexB = clothGP->mesh->indices[i + 1];
        size_t indexC = clothGP->mesh->indices[i + 2];

        CranePhysics::Rigidbody &a = *(clothPhys[indexA]);
        CranePhysics::Rigidbody &b = *(clothPhys[indexB]);
        CranePhysics::Rigidbody &c = *(clothPhys[indexC]);
        float distAB = (a.position - b.position).norm();
        pbd->constraints.emplace_back(std::make_shared<CranePhysics::Stretching>(a, b, distAB, compress, stretch));

        float distAC = (a.position - c.position).norm();
        pbd->constraints.emplace_back(std::make_shared<CranePhysics::Stretching>(a, c, distAC, compress, stretch));

        float distBC = (b.position - c.position).norm();
        pbd->constraints.emplace_back(std::make_shared<CranePhysics::Stretching>(b, c, distBC, compress, stretch));
    }
}

void ClothActor::update()
{
    clothGP->mesh->setVertices([this](uint32_t i, Crane::Vertex &v)
                               { v.position = this->clothPhys[i]->position - this->clothGP->transformer.getPosition(); });
    clothGP->mesh->recomputeNormals();

    for (size_t i = 0; i < clothGP->mesh->data.size(); i++)
        ctx->vertices[clothGP->vertBufferOffset + i] = clothGP->mesh->data[i];
    ctx->vertBuff.update(ctx->vertices.data());

    sphereGP->transformer.setPosition(spherePhy->position);
    sphereAGP->transformer.setPosition(sphereAPhy->position);
    sphereTestGP->transformer.setPosition(sphereTestPhy->position);
}
