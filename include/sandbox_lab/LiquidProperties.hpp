#pragma once

#include "MaterialProperties.hpp"

namespace sandbox
{
    struct LiquidProperties {
        float gravity;
        float maxVelocity;
        float verticalFlow;
        float diagonalFlow;
        float horizontalFlow;
        float minImpactSpeed;
        float impactSpeedPerPass;
        int maxImpactPasses;
    };

    inline constexpr LiquidProperties getLiquidProperties(Material material) {
        switch (material) {
            case Material::Water:
                return {0.5f, 6.0f, 1.0f, 1.0f, 1.0f, 2.0f, 1.5f, 3};
            case Material::Oil:
                return {0.35f, 4.0f, 1.0f, 0.6f, 0.5f, 2.0f, 1.5f, 3};
            case Material::Lava:
                return {0.2f, 2.5f, 1.0f, 0.35f, 0.25f, 2.0f, 1.5f, 3};
            default:
                return {};
        }
    }
} // namespace sandbox
