#include "MINI_GAME.hpp"

using namespace std;
using namespace Eigen;
using namespace Crane;
using namespace CranePhysics;


void MINI_GAME::updatePhysics()
{
	dt = 0.01f;
	pbd.dt =dt;
	//pbd.rigidbodies[1]->rotation *= Quaternionf(AngleAxisf(1.f*dt, Vector3f(1.0f, 1.0f, 1.0f).normalized()));
	Vector3f u{ 2.f, 0.f, 2.f };
	float drag = 1.f;
	float lift = 1.f;

	for(size_t i = 0; i < cloak.mesh->indices.size(); i = i + 3)
	{
		size_t indexA = cloak.mesh->indices[i];
		size_t indexB = cloak.mesh->indices[i+1];
		size_t indexC = cloak.mesh->indices[i+2];
		CranePhysics::Particle &a = *dynamic_cast<Particle*>(pbd.rigidbodies[offsetCloak+indexA].get());
		CranePhysics::Particle &b = *dynamic_cast<Particle*>(pbd.rigidbodies[offsetCloak+indexB].get());
		CranePhysics::Particle &c = *dynamic_cast<Particle*>(pbd.rigidbodies[offsetCloak+indexC].get());

		Vector3f AB = a.positionPrime - b.positionPrime;
		Vector3f AC = a.positionPrime - c.positionPrime;
		Vector3f ABC = AB.cross(AC);
		float area = ABC.norm() / 2;

		Vector3f vRel = (a.velocity + b.velocity + c.velocity)/3 - u;
		Vector3f n = ABC.normalized();
		n = vRel.dot(n) > 0 ? n : -n;

		Vector3f fd = drag * vRel.squaredNorm() * area * vRel.dot(n) * -vRel;
		float cosq = vRel.normalized().cross(n).norm();
		Vector3f fl = lift * vRel.squaredNorm() * area * cosq * (n.cross(vRel).cross(vRel));

		a.velocity += dt * a.invMass *(fd+fl);

		b.velocity += dt * b.invMass *(fd+fl);

		c.velocity += dt * c.invMass *(fd+fl);
	}

	pbd.run();

	//ball.setPosition(pbd.rigidbodies[offsetBall]->position);

	cloak.mesh->setVertices([this](uint32_t i, Crane::Vertex &v){
		Vector4f homogeneous = cloak.transform.inverse() * Vector4f{ this->pbd.rigidbodies[offsetCloak + i]->position[0],
								 this->pbd.rigidbodies[offsetCloak + i]->position[1],
								 this->pbd.rigidbodies[offsetCloak + i]->position[2],
								 1.0f
		};

		v.position = Vector3f{homogeneous[0], homogeneous[1], homogeneous[2]};
		});
	cloak.mesh->recomputeNormals();
}
