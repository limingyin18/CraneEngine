#pragma once

#include "Engine.hpp"

/**
 * @brief Cloth actor class 
 * 
 */
class ClothActor : public Crane::Actor
{
private:
    /* data */

public:
    void init(Crane::Engine * ctx);
    ClothActor(/* args */) = default;
    ~ClothActor() = default;
};