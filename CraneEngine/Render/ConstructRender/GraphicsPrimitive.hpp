#pragma once

#include "MeshBase.hpp"
#include "MaterialSystem.hpp"
#include "Transformer.hpp"

namespace Crane
{
	class GraphicsPrimitive
	{
	public:
		std::shared_ptr<Crane::MeshBase> mesh = nullptr;
		std::shared_ptr<Crane::Material> material = nullptr; 
		Transformer transformer;

		Eigen::Vector3f extentMin, extentMax;
		void computeAABB();

		Eigen::Vector4f sphereBound;
		Eigen::Vector4f SphereBound();

		GraphicsPrimitive() = default;
	};
}