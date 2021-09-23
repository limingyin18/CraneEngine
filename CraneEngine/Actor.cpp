#include "Actor.hpp"

using namespace std;
using namespace Eigen;
using namespace Crane;

void Actor::setPosition(Eigen::Vector3f p)
{
	position = p;

	transform = (Translation3f(position) * rotationQ).matrix();
}

void Crane::Actor::setRotation(Eigen::Vector3f r)
{
	rotation = r;
	auto aa = AngleAxisf(rotation[0], Vector3f::UnitX());
	rotationQ = Quaternionf(aa);
	auto bb = AngleAxisf(rotation[1], Vector3f::UnitY());
	rotationQ *= Quaternionf(bb);
	auto cc = AngleAxisf(rotation[0], Vector3f::UnitZ());
	rotationQ *= Quaternionf(aa);

	transform = (Translation3f(position) * rotationQ).matrix();
}
