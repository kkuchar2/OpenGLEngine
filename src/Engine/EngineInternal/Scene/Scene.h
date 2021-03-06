#pragma once

#include "GameObject/GameObject.h"
#include <Engine/EngineInternal/Rendering/Projection.h>

class Scene {
    public:

        Projection projection = PERSPECTIVE;

        std::vector<std::shared_ptr<GameObject>> children;

        Scene() = default;

        void addChild(const std::shared_ptr<GameObject> & child) {
            children.push_back(child);
        }
};