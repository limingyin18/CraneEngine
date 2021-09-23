#pragma once

#include <numbers>
#include <vector>
#include <functional>

#include <vulkan/vulkan.hpp>
#include <Eigen/Eigen>

namespace Crane
{
    /**
     * @brief ����ṹ
     */
    struct Vertex
    {
        Eigen::Vector3f position{0.f, 0.f, 0.f}; // λ��
        Eigen::Vector3f normal{1.f, 1.f, 1.f};   // ����
        Eigen::Vector3f color{ 1.f, 1.f, 1.f };  // ��ɫ
        Eigen::Vector2f uv{ 0.f, 0.f };          // ����

        static std::vector<vk::VertexInputBindingDescription> GetVertexInputBindingDescription();

        static std::vector<vk::VertexInputAttributeDescription> GetVertexInputAttributeDescription();

        static vk::PipelineInputAssemblyStateCreateFlags GetPipelineInputAssemblyStateCreateFlags();
    };

    /**
     * @brief �������
     */
    class MeshBase
    {
    public:
        std::vector<Vertex> data;      // ��������
        std::vector<uint32_t> indices; // ��������

        /**
         * @brief ���ö���
         *
         * @param fun ���ú���
         */
        void setVertices(std::function<void(uint32_t, Vertex &)> const &fun);

        /**
         * @brief ���߼���
         * @details ���������߲�˼��㷨�ߣ������������������ռ��
         */
        void recomputeNormals();
    };

    /**
     * @brief ƽ��
     */
    class Plane : public MeshBase
    {
    public:
        explicit Plane(uint32_t n, uint32_t m);
    };

    /**
     * @brief ������
     */
    class Cube : public MeshBase
    {
    public:
        explicit Cube(int n);
    };

    /**
     * @brief ����
     */
    class Chessboard : public MeshBase
    {
    public:
        explicit Chessboard(uint32_t n, uint32_t m);
    };

    /**
     * @brief ����
     */
    class Sphere : public MeshBase
    {
    public:
        explicit Sphere(uint32_t lng, uint32_t lat, float radius = 1.f);
    };

}
