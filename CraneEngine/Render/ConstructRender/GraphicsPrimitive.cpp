#include "GraphicsPrimitive.hpp"

using namespace std;
using namespace Eigen;
using namespace Crane;

void GraphicsPrimitive::computeAABB()
{
    extentMin = Vector3f(numeric_limits<float>::max(), numeric_limits<float>::max(), numeric_limits<float>::max());
    extentMax = Vector3f(-numeric_limits<float>::max(), -numeric_limits<float>::max(), -numeric_limits<float>::max());

    for (const Vertex &v : mesh->data)
    {
        const Vector3f &p = v.position;

        extentMin = p.cwiseMin(extentMin);
        extentMax = p.cwiseMax(extentMax);
    }
}

Eigen::Vector4f GraphicsPrimitive::SphereBound()
{
    unsigned int maxX = 0, maxY = 0, maxZ = 0, minX = -1, minY = -1, minZ = -1;
    float maxXPos = std::numeric_limits<float>::min();
    float minXPos = std::numeric_limits<float>::max();
    float maxYPos = std::numeric_limits<float>::min();
    float minYPos = std::numeric_limits<float>::max();
    float maxZPos = std::numeric_limits<float>::min();
    float minZPos = std::numeric_limits<float>::max();
    // Find the max and min along the x-axie, y-axie, z-axie
    for (int i = 0; i < mesh->data.size(); i++)
    {
        if (mesh->data[i].position.x() > maxXPos)
        {
            maxXPos = mesh->data[i].position.x();
            maxX = i;
        }
        if (mesh->data[i].position.x() < minXPos)
        {
            minXPos = mesh->data[i].position.x();
            minX = i;
        }
        if (mesh->data[i].position.y() > maxYPos)
        {
            maxYPos = mesh->data[i].position.y();
            maxY = i;
        }
        if (mesh->data[i].position.y() < minYPos)
        {
            minYPos = mesh->data[i].position.y();
            minY = i;
        }
        if (mesh->data[i].position.z() > maxZPos)
        {
            maxZPos = mesh->data[i].position.z();
            maxZ = i;
        }
        if (mesh->data[i].position.z() < minZPos)
        {
            minZPos = mesh->data[i].position.z();
            minZ = i;
        }
    } // end for

    float x = (mesh->data[minX].position - mesh->data[maxX].position).norm();
    float y = (mesh->data[minY].position - mesh->data[maxY].position).norm();
    float z = (mesh->data[minZ].position - mesh->data[maxZ].position).norm();

    float dia = x;
    int max = maxX, min = minX;
    if (z > x && z > y)
    {
        max = maxZ;
        min = minZ;
        dia = z;
    }
    else if (y > x && y > z)
    {
        max = maxY;
        min = minY;
        dia = y;
    }

    // Compute the center point
    Eigen::Vector3f center;
    center.x() = 0.5 * (mesh->data[max].position.x() + mesh->data[min].position.x());
    center.y() = 0.5 * (mesh->data[max].position.y() + mesh->data[min].position.y());
    center.z() = 0.5 * (mesh->data[max].position.z() + mesh->data[min].position.z());

    // Compute the radious
    float radius = 0.5 * sqrt(dia);

    // Fix it
    for (int i = 0; i < mesh->data.size(); i++)
    {
        Eigen::Vector3f d = mesh->data[i].position - center;
        float dist = d.norm();

        if (dist > radius)
        {
            float newRadious = (dist + radius) * 0.5;
            float sp = newRadious - radius;
            radius = newRadious;
            center = center + sp * d.normalized();
        }
    }

    return Eigen::Vector4f(center.x(), center.y(), center.z(), radius);
};