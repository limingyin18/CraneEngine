#pragma once

#include <memory>

#include "MeshBase.hpp"
#include "MaterialSystem.hpp"

namespace Crane
{
	struct RenderableBase
	{
		RenderableBase(MeshBase* m = nullptr, Material* ma = {}, Eigen::Matrix4f t = Eigen::Matrix4f::Identity())
			: mesh{ m }, material{ ma }, transformMatrix{ t } {};

		MeshBase* mesh;
		Material* material;
		Eigen::Matrix4f transformMatrix = Eigen::Matrix4f::Identity();
	};
}