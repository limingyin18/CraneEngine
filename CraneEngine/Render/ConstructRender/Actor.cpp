#include "Actor.hpp"

using namespace Crane;

void Actor::updateTransform() 
{
    for(auto gp:primitives)
        gp.transformer.update();

    for(auto child : childs)
    {
        child->updateTransform();
    }
}