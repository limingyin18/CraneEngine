#pragma once

#include <vector>
#include <functional>

#include <vulkan/vulkan.hpp>
#include <Eigen/Eigen>

namespace Crane
{
    struct Vertex
    {
        Eigen::Vector3f position;
        Eigen::Vector3f normal;
        Eigen::Vector3f color;
        Eigen::Vector2f uv;

        Vertex(Eigen::Vector3f pos = {0.f, 0.f, 0.f}, Eigen::Vector3f n={1.f, 1.f, 1.f}, Eigen::Vector3f c= {1.f, 1.f, 1.f}, Eigen::Vector2f u={0.f, 0.f});

        static std::vector<vk::VertexInputBindingDescription> GetVertexInputBindingDescription();

        static std::vector<vk::VertexInputAttributeDescription> GetVertexInputAttributeDescription();

        static vk::PipelineInputAssemblyStateCreateFlags GetPipelineInputAssemblyStateCreateFlags();
    };

    class MeshBase
    {
    public:
        std::vector<Vertex> data;
        std::vector<uint32_t> indices;

        void setVertices(std::function<void(uint32_t, Vertex &)> const &fun);
        virtual void recomputeNormals(std::vector<Vertex> &data);
    };

    class Plane : public MeshBase
    {
    public:
        explicit Plane(uint32_t n, uint32_t m);
    };

    class Cube : public MeshBase
    {
    public:
        explicit Cube(int n);
    };

}
