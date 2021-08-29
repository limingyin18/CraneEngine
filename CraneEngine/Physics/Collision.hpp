#pragma once

#include <optional>

#include <Eigen/Eigen>

namespace CranePhysics
{
    class Rigidbody;
    class Particle;
    class Cube;
    class ParticleCollider;
    class CubeCollider;
    class PositionBasedDynamics;

    void CollisionDetectDispatch(Rigidbody *a, Rigidbody *b, PositionBasedDynamics &pbd);
    std::optional<std::pair<Eigen::Vector3f, float>> CollisionDetectFunction(const Particle &a, const Cube &b);
    std::optional<std::pair<Eigen::Vector3f, float>> CollisionDetectFunction(const Particle &a, const Particle &b);

    /**
     * 碰撞器接口
     */
    class Collider
    {
    public:
        Collider(Rigidbody *r) : rb{r} {};

        Rigidbody *rb;

        virtual void ColliderDispatch(Collider *b, PositionBasedDynamics &pbd) = 0;
        virtual void ColliderDispatch(ParticleCollider *B, PositionBasedDynamics &pbd) = 0;
        virtual void ColliderDispatch(CubeCollider *B, PositionBasedDynamics &pbd) = 0;
    };

    /**
     * 球体碰撞器
     */
    class ParticleCollider : public Collider
    {
    public:
        ParticleCollider(Particle *a);

        void ColliderDispatch(Collider *b, PositionBasedDynamics &pbd)
        {
            b->ColliderDispatch(this, pbd);
        }

        void ColliderDispatch(ParticleCollider *B, PositionBasedDynamics &pbd) override;

        void ColliderDispatch(CubeCollider *B, PositionBasedDynamics &pbd) override;
    };

    /**
     * 长方体碰撞器
     */
    class CubeCollider : public Collider
    {
    public:
        CubeCollider(Cube *a);

        void ColliderDispatch(Collider *b, PositionBasedDynamics &pbd)
        {
            b->ColliderDispatch(this, pbd);
        }
        void ColliderDispatch(ParticleCollider *B, PositionBasedDynamics &pbd) override;

        void ColliderDispatch(CubeCollider *B, PositionBasedDynamics &pbd) override
        {
        }
    };

}