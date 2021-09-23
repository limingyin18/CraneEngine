#include "Intersect.hpp"

using namespace std;
using namespace Eigen;
using namespace CranePhysics;

optional<pair<Vector3f, float>> CranePhysics::SphereCubeIntersect(
    Vector3f bmin, Vector3f bmax, Vector3f c, Eigen::Quaternionf t, Vector3f p, float r)
{
    // ת��Բ��p������������ϵ
    Vector3f pc = p - c;
    Quaternionf pcQ = t.inverse() * Quaternionf(0, pc.x(), pc.y(), pc.z()) * t;
    Vector3f p1{pcQ.x(), pcQ.y(), pcQ.z()};

    // ����������ӽ�Բ�ĵĵ�q
    Vector3f q1 = p1.cwiseMax(bmin).cwiseMin(bmax);

    // ת�������q����������ϵ
    Quaternionf qQ = t * Quaternionf(0, q1.x(), q1.y(), q1.z()) * t.inverse();
    Vector3f q{qQ.x(), qQ.y(), qQ.z()};
    q += c;

    // �����嵽Բ�ĵ����ʸ��
    Vector3f u = p - q;

    if (u.squaredNorm() <= r * r)
        return pair{u, r - u.norm()};
    else
        return nullopt;
}

std::optional<std::pair<Eigen::Vector3f, float>> CranePhysics::SphereSphereIntersect(
    Eigen::Vector3f c1, float r1, Eigen::Vector3f c2, float r2)
{
    Vector3f d = c1-c2;
    float r = r1 + r2;
    if(d.squaredNorm() <= r*r)
        return pair{d, r - d.norm()};
    else
        return nullopt;
}