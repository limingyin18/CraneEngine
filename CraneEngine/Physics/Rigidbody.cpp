#include "Rigidbody.hpp"

using namespace CranePhysics;

Rigidbody::Rigidbody(float inv) : invMass{inv}
{
}

Particle::Particle(float inv) : Rigidbody(inv)
{
}

Cube::Cube(float inv) : Rigidbody(inv)
{
}