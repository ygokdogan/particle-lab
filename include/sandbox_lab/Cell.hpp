#pragma once

#include "Material.hpp"

namespace sandbox
{
    struct Cell {
        Material material = Material::Empty;
        std::uint64_t lastUpdatedFrame = 0;
    };
} 


