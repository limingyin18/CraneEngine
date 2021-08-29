#include "MeshBase.hpp"

using namespace std;
using namespace Crane;
using namespace Eigen;

Vertex::Vertex(Eigen::Vector3f pos, Eigen::Vector3f n, Eigen::Vector3f c, Eigen::Vector2f u)
    : position(pos), normal(n), color(c), uv(u)
{
}

vector<vk::VertexInputBindingDescription> Vertex::GetVertexInputBindingDescription()
{
    vector<vk::VertexInputBindingDescription> bindings;
    vk::VertexInputBindingDescription mainBinding;
    mainBinding.binding = 0;
    mainBinding.stride = sizeof(Vertex);
    mainBinding.inputRate =vk::VertexInputRate::eVertex;

    bindings.push_back(mainBinding);

    return bindings;
}

vector<vk::VertexInputAttributeDescription> Vertex::GetVertexInputAttributeDescription()
{
    vector<vk::VertexInputAttributeDescription> attributes;

    // position
    VkVertexInputAttributeDescription positionAttribute = {};
    positionAttribute.binding = 0;
    positionAttribute.location = 0;
    positionAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
    positionAttribute.offset = offsetof(Vertex, position);

    // normal
    VkVertexInputAttributeDescription normalAttribute = {};
    normalAttribute.binding = 0;
    normalAttribute.location = 1;
    normalAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
    normalAttribute.offset = offsetof(Vertex, normal);

    // color
    VkVertexInputAttributeDescription colorAttribute = {};
    colorAttribute.binding = 0;
    colorAttribute.location = 2;
    colorAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
    colorAttribute.offset = offsetof(Vertex, color);

    // uv
    VkVertexInputAttributeDescription uvAttribute = {};
    uvAttribute.binding = 0;
    uvAttribute.location = 3;
    uvAttribute.format = VK_FORMAT_R32G32_SFLOAT;
    uvAttribute.offset = offsetof(Vertex, uv);

    attributes.push_back(positionAttribute);
    attributes.push_back(normalAttribute);
    attributes.push_back(colorAttribute);
    attributes.push_back(uvAttribute);

    return attributes;
}

vk::PipelineInputAssemblyStateCreateFlags Vertex::GetPipelineInputAssemblyStateCreateFlags()
{
    return {};
}

void MeshBase::setVertices(function<void(unsigned, Vertex &)> const &fun)
{
    for (unsigned i = 0; i < data.size(); ++i)
    {
        fun(i, data[i]);
    }
}

void MeshBase::recomputeNormals(vector<Vertex> &data)
{
    for (auto &d : data)
    {
        d.normal = {0.f, 0.f, 0.f};
    }

    for (unsigned i = 0; i < indices.size();)
    {
        auto id1 = indices[i++];
        auto id2 = indices[i++];
        auto id3 = indices[i++];

        auto v1 = data[id1].position;
        auto v2 = data[id2].position;
        auto v3 = data[id3].position;

        // This does weighted area based on triangle area
        auto n = (v2 - v1).cross(v3 - v1);

        data[id1].normal = data[id1].normal + n;
        data[id2].normal = data[id2].normal + n;
        data[id3].normal = data[id3].normal + n;
    }

    for (auto &d : data)
    {
        d.normal.normalize();
    }
}

Plane::Plane(unsigned nx, unsigned nz)
{
    assert(nx > 1 && nz > 1);
    data.reserve((nx) * (nz));
    indices.reserve(6 * (nx - 1) * (nz - 1));

    const float dx = 2.0f / (nx - 1);
    const float dz = 2.0f / (nz - 1);

    for (unsigned i = 0; i < nx; ++i)
    {
        for (int j = nz - 1; j >= 0; --j)
        {
            data.emplace_back(Vector3f{-1.0f + i * dx, 0.0f, -1.0f + j * dz},
                              Vector3f{0.0f, 1.0f, 0.0f},
                              Vector3f{1.0f, 1.0f, 1.0f},
                              Vector2f{static_cast<float>(i) / static_cast<float>(nx-1),
                                       static_cast<float>(j) / static_cast<float>(nz-1)});
        }
    }

    for (unsigned i = 0; i < nx - 1; ++i)
    {
        for (unsigned j = 0; j < nz - 1; ++j)
        {
            const unsigned I = i * nz + j;
            indices.emplace_back(I);
            indices.emplace_back(I + nz);
            indices.emplace_back(I + 1);

            indices.emplace_back(I + 1);
            indices.emplace_back(I + nz);
            indices.emplace_back(I + nz + 1);
        }
    }
}

Cube::Cube(int n)
{
    assert(n > 1);
    data.reserve(6 * n * n);
    indices.reserve(6 * 6 * (n - 1) * (n - 1));

    const float dx = 2.0f / (n - 1);

    // bottom
    for (int i = 0; i < n; ++i)
    {
        for (int j = 0; j < n; ++j)
        {
            data.emplace_back(Vector3f{-1.0f + i * dx, 1.0f, -1.0f + j * dx},
                              Vector3f{0.0f, -1.0f, 0.0f},
                              Vector3f{1.0f, 1.0f, 1.0f},
                              Vector2f{1.0f - static_cast<float>(i) / static_cast<float>(n-1),
                                       1.0f - static_cast<float>(j) / static_cast<float>(n-1)});
        }
    }

    // top
    for (int i = 0; i < n; ++i)
    {
        for (int j = n - 1; j >= 0; --j)
        {
            data.emplace_back(Vector3f{-1.0f + i * dx, -1.0f, -1.0f + j * dx},
                              Vector3f{0.0f, 1.0f, 0.0f},
                              Vector3f{1.0f, 1.0f, 1.0f},
                              Vector2f{1.0f - static_cast<float>(i) / static_cast<float>(n-1),
                                       static_cast<float>(j) / static_cast<float>(n-1)});
        }
    }

    // back
    for (int i = 0; i < n; ++i)
    {
        for (int j = 0; j < n; ++j)
        {
            data.emplace_back(Vector3f{-1.0f + i * dx, -1.0f + j * dx, -1.0f},
                              Vector3f{0.0f, 0.0f, 1.0f},
                              Vector3f{1.0f, 1.0f, 1.0f},
                              Vector2f{static_cast<float>(i) / static_cast<float>(n-1),
                                       1.0f - static_cast<float>(j) / static_cast<float>(n-1)});
        }
    }

    // front
    for (int i = 0; i < n; ++i)
    {
        for (int j = n - 1; j >= 0; --j)
        {
            data.emplace_back(Vector3f{-1.0f + i * dx, -1.0f + j * dx, 1.0f},
                              Vector3f{0.0f, 0.0f, -1.0f},
                              Vector3f{1.0f, 1.0f, 1.0f},
                              Vector2f{static_cast<float>(i) / static_cast<float>(n-1),
                                       static_cast<float>(j) / static_cast<float>(n-1)});
        }
    }

    // left
    for (int i = 0; i < n; ++i)
    {
        for (int j = 0; j < n; ++j)
        {
            data.emplace_back(Vector3f{-1.0f, -1.0f + i * dx, -1.0f + j * dx},
                              Vector3f{-1.0f, 0.0f, 0.0f},
                              Vector3f{1.0f, 1.0f, 1.0f},
                              Vector2f{1.0f - static_cast<float>(j) / static_cast<float>(n-1),
                                       static_cast<float>(i) / static_cast<float>(n-1)});
        }
    }

    // right
    for (int i = 0; i < n; ++i)
    {
        for (int j = n - 1; j >= 0; --j)
        {
            data.emplace_back(Vector3f{1.0f, -1.0f + i * dx, -1.0f + j * dx},
                              Vector3f{1.0f, 0.0f, 0.0f},
                              Vector3f{1.0f, 1.0f, 1.0f},
                              Vector2f{static_cast<float>(j) / static_cast<float>(n-1),
                                       static_cast<float>(i) / static_cast<float>(n-1)});
        }
    }

    // indices
    for (int k = 0; k < 6; ++k)
    {
        for (int i = 0; i < n - 1; ++i)
        {
            for (int j = 0; j < n - 1; ++j)
            {
                const unsigned I = i * n + j + k * n * n;
                indices.emplace_back(I);
                indices.emplace_back(I + 1);
                indices.emplace_back(I + n);

                indices.emplace_back(I + n);
                indices.emplace_back(I + 1);
                indices.emplace_back(I + n + 1);
            }
        }
    }
}
