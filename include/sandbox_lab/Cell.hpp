#pragma once

#include <cstdint>
#include <vector>

#include "Material.hpp"

namespace sandbox
{
    struct LiquidLayer {
        Material material;
        float amount;
    };

    struct Cell {
        Material solid = Material::Empty;
        std::vector<LiquidLayer> liquids;
        std::uint64_t lastUpdatedFrame = 0;

        Material gas = Material::Empty;
        std::uint16_t gasLifetime = 0;
        std::uint16_t lastGasUpdatedFrame = 0;
        
        std::uint16_t burnTicks = 0;
        std::uint16_t burnDuration = 0;
    };
}