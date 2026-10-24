#pragma once

#include "Material.hpp"
#include "Color.hpp"

namespace sandbox {
    enum class Behaviour {
        None,
        Powder,
        Liquid,
        Gas,
        Static,
    };

    struct MaterialProperties {
        Behaviour behaviour;
        RGB color;
        float density;
    };

    const MaterialProperties &getMaterialProperties(Material material);
}