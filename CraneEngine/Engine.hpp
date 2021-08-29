#pragma once

#include <chrono>

#include "Render/Render.hpp"
#include "Input.hpp"
#include "Utils.hpp"
#include "Physics/PositionBasedDynamics.hpp"
#include "Physics/Constraint.hpp"

#include "asset_loader.h"
#include "texture_asset.h"
#include "mesh_asset.h"
#include "prefab_asset.h"
#include "material_asset.h"

namespace Crane
{
    /**
     * engine base class
     */
    class Engine : public Render
    {
    public:
        explicit Engine();
        virtual ~Engine() = default;
        Engine(const Engine& rhs) = delete;
        Engine(Engine&& rhs) = delete;
        Engine& operator=(const Engine& rhs) = delete;
        Engine& operator=(Engine&& rhs) = delete;

        void updateEngine();
        void updateInput();
        Input input;

    protected:
        std::chrono::time_point<std::chrono::system_clock> mTime =
            std::chrono::system_clock::now();
        float dtAll = 0.0f;
        float dt = 0.0f;
    };
}