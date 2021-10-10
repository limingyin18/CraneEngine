#include "MINI_GAME.hpp"

using namespace std;
using namespace Eigen;
using namespace Crane;

void MINI_GAME::updateAI()
{
	//soldiers[0].transform.block<3, 1>(0, 3) = Vector3f{ 10*sin(dtAll), 0.f, 10*cos(dtAll) };

	for (uint32_t i = 0; i < soldierCount; ++i)
	{
		std::for_each(execution::par, soldiers.begin() + (i * soldierCount), soldiers.begin() + (i * soldierCount) + soldierCount, [&i, this](auto &s) {
			Vector3f direction = Vector3f{ float(0), 0.f, float(i) } - s.position;
			float speed = 10.0f;
			if (direction.norm() > 0.1f)
			{
				direction.normalize();
				s.setPosition(s.position + direction * dt * speed);
			}
			});
	}
}