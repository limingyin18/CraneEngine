#include "Transformer.hpp"

using namespace Eigen;
using namespace Crane;

Transformer::Transformer(const Eigen::Vector3f &p,
                         const Eigen::Quaternionf &q,
                         const Eigen::Vector3f &s)
    : position{p}, quaternion{q}, scale{s}
{
    update();
}

Transformer::Transformer(const Eigen::Vector3f &p,
                         const Eigen::Vector3f &r,
                         const Eigen::Vector3f &s)
    : position{p}, scale{s}
{
    setRotation(r);
}

void Transformer::update()
{
    transform = (Translation3f(position) * quaternion * Scaling(scale)).matrix();
    if(transformParent)
        transformWorld = *transformParent * transform;
    else
        transformWorld = transform;
}

void Transformer::setPosition(const Eigen::Vector3f &p)
{
    position = p;
    update();
}

void Transformer::setRotation(const Eigen::Vector3f &r)
{
    quaternion = AngleAxisf(r[0], Vector3f::UnitX()) *
                 AngleAxisf(r[1], Vector3f::UnitY()) *
                 AngleAxisf(r[2], Vector3f::UnitZ());
    update();
}

void Transformer::setScale(const Eigen::Vector3f &s)
{
    scale = s;
    update();
}

void Transformer::setQuaternion(const Eigen::Quaternionf &q)
{
    quaternion = q;
    update();
}

void Transformer::setTransformParent(const Eigen::Matrix4f *transP)
{
    transformParent = transP;
    update();
}