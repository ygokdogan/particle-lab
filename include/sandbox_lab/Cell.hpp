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
    };
}