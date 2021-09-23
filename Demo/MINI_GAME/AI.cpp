#include "MINI_GAME.hpp"

using namespace std;
using namespace Eigen;
using namespace Crane;

void MINI_GAME::updateAI()
{
	//soldiers[0].transform.block<3, 1>(0, 3) = Vector3f{ 10*sin(dtAll), 0.f, 10*cos(dtAll) };
	float speed = 10.0f;

	for (uint32_t i = 0; i < soldierCount; ++i)
	{
		for (uint32_t j = 0; j < soldierCount; ++j)
		{
			auto& s = soldiers[i*soldierCount+j];
			Vector3f direction = Vector3f{ float(j), 0.f, float(i) } - s.position;
			if (direction.norm() > 0.1f)
			{
				direction.normalize();
				s.setPosition(s.position + direction * dt * speed);
			}
		}
	}
}