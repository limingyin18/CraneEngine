#pragma once

#include "Engine.hpp"

namespace Crane
{
	class Boat : public Crane::Actor
	{
	public:
		void init(Crane::Render* ctx) override;

		void updatePhysics();

		float yMin();
		float bottomHeight;
		float seaHeight;
		float Area();
		float area;

		const float rho = 1000.f;
		const float g = 10.f;
		const float m = 10000.f;
		//float boatArea = 10.f;
		//float bottom = boat.position.y() - boatHalfHeight;
		//float diffH = 0.f;
		//if (h > bottom)
		//{
		//	diffH = h - bottom;
		//}

		//float bf = diffH * boatArea * 1000.f * 10.f;
		//float G = 100.f * 10;
		//float f = bf - G;
		//boat.position.y() += f / 100.f * dt * dt / 2.f;

		//Vector3f position = { boat.position.x(), h, boat.position.z() };
		//boat.setPosition(position);
	};
}