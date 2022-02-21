#pragma once

#include <Eigen/Eigen>

namespace Crane
{
    /**
     * @brief postion rotaion scale
     *
     */
    class Transformer
    {
    private:
        /* data */
        Eigen::Vector3f position;
        Eigen::Quaternionf quaternion;
        Eigen::Vector3f scale;
        Eigen::Matrix4f transform = Eigen::Matrix4f::Identity();
        const Eigen::Matrix4f *transformParent = nullptr;
        Eigen::Matrix4f transformWorld = Eigen::Matrix4f::Identity();


    public:
        Transformer(const Eigen::Vector3f &p = Eigen::Vector3f::Zero(),
                    const Eigen::Quaternionf &q = Eigen::Quaternionf::Identity(),
                    const Eigen::Vector3f &s = Eigen::Vector3f::Ones());

        Transformer(const Eigen::Vector3f &p,
                    const Eigen::Vector3f &r,
                    const Eigen::Vector3f &s);

        void update();

        const Eigen::Vector3f &getPosition() { return position; };
        const Eigen::Quaternionf &getQuaternion() { return quaternion; };
        const Eigen::Vector3f &getRotation() { return std::move(quaternion.toRotationMatrix().eulerAngles(0, 1, 2)); };
        const Eigen::Vector3f &getScale() { return scale; };
        const Eigen::Matrix4f &getTransform() { return transform; };
        const Eigen::Matrix4f *getTransformParent() { return transformParent; };
        const Eigen::Matrix4f &getTransformWorld() { return transformWorld; };

        void setPosition(const Eigen::Vector3f &p);
        void setQuaternion(const Eigen::Quaternionf &q);
        void setRotation(const Eigen::Vector3f &r);
        void setScale(const Eigen::Vector3f &s);
        void setTransformParent(const Eigen::Matrix4f *transP);
    };
}