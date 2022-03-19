#pragma once

#include "Transformer.hpp"
#include "GraphicsPrimitive.hpp"

/**
 * @brief basic actor in engine
 *
 */
namespace Crane
{
    class Actor
    {
    public:
        Transformer transformer;
        std::vector<std::shared_ptr<GraphicsPrimitive>> primitives;
        std::vector<std::shared_ptr<Actor>> childs;

        void updateTransform();
    };
}