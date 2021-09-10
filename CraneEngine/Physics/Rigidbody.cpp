#include "Rigidbody.hpp"

using namespace CranePhysics;

Rigidbody::Rigidbody(float inv) : invMass{inv}
{
}

Particle::Particle(float inv) : Rigidbody(inv), collider(this)
{
}

Cube::Cube(float inv) : Rigidbody(inv), collider(this)
{
}