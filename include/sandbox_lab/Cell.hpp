#pragma once

#include <cstdint>
#include <vector>

#include "Material.hpp"

namespace sandbox {
    struct LiquidLayer {
        Material material;
        float amount;
        float velocityY = 0.0f;
    };

    struct Cell {
        std::uint64_t lastUpdatedFrame = 0;
        
        float velocityY = 0.0f;
        float movementRemainder = 0.0f;

        Material solid = Material::Empty;
        std::vector<LiquidLayer> liquids;

        Material gas = Material::Empty;
        std::uint16_t gasLifetime = 0;
        std::uint16_t lastGasUpdatedFrame = 0;

        std::uint16_t burnTicks = 0;
        std::uint16_t burnDuration = 0;

        std::uint16_t coolingTicks = 0;
    };
} // namespace sandbox