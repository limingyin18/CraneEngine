#pragma once

#include "Render.hpp"

namespace Crane
{
	class Actor
	{
	public:
		virtual void init(Render* ctx) { context = ctx; };
		Render* context = nullptr;
		std::shared_ptr<Crane::MeshBase> mesh = nullptr;
		Crane::Material *material = nullptr; 

		Eigen::Vector3f position = Eigen::Vector3f::Zero();
		Eigen::Vector3f rotation = Eigen::Vector3f::Zero();
		Eigen::Vector3f scale = Eigen::Vector3f::Ones();
		Eigen::Quaternionf rotationQ = Eigen::Quaternionf::Identity();
		Eigen::Matrix4f transform = Eigen::Matrix4f::Identity();

		void setPosition(Eigen::Vector3f p);
		void setRotation(Eigen::Vector3f r);
		void setScale(Eigen::Vector3f s);

		// aabb

		Eigen::Vector3f extentMin, extentMax;

		void computeAABB();
	};
}