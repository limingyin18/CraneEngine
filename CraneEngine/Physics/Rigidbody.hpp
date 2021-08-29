#pragma once

#include <vector>
#include <optional>
#include <Eigen/Eigen>

#include "Collision.hpp"

using namespace Eigen;

namespace CranePhysics
{
    /**
     * 刚体
     */
    class Rigidbody
    {
    public:
        float invMass = 0.f;

        Vector3f velocity = Vector3f{0.f, 0.f, 0.f};
        Vector3f position = Vector3f{0.f, 0.f, 0.f};
        Quaternionf rotation = Quaternionf{1.0f, 0.f, 0.f, 0.f};

        Vector3f velocityPrime = Vector3f{0.f, 0.f, 0.f};
        Vector3f positionPrime = Vector3f{0.f, 0.f, 0.f};

    public:
        explicit Rigidbody(float inv = 1.f);
        virtual ~Rigidbody() = default;

        virtual Collider* getCollider() = 0;
    };

    /**
     * 粒子/球体
     */
    class Particle : public Rigidbody
    {
    public:
        explicit Particle(float inv = 1.f);
        ~Particle() = default;

        ParticleCollider* getCollider() override
        {
            auto *collider = new ParticleCollider(this);
            return collider;
        };

        float radius = 1.0f;
    };

    using Sphere = Particle;

    /**
     * 长方体
     */
    class Cube : public Rigidbody
    {
    public:
        explicit Cube(float inv = 1.f);
        ~Cube() = default;

        CubeCollider* getCollider() override
        {
            return new CubeCollider(this);
        };

        float width = 1.0f;
        float height = 1.0f;
        float depth = 1.0f;
    };
}
