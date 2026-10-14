#include "sandbox_lab/MaterialProperties.hpp"

namespace sandbox {
    const MaterialProperties& getMaterialProperties(Material material) {
        static const MaterialProperties emptyProperties {
            Behaviour::None,
            {0, 0, 0},
            0.0f
        };
        static const MaterialProperties sandProperties {
            Behaviour::Powder,
            {230, 200, 90},
            1600.0f
        };
        static const MaterialProperties waterProperties {
            Behaviour::Liquid,
            {50, 120, 230},
            1000.0f
        };

        switch (material) {
            case Material::Empty:
                return emptyProperties;
            case Material::Sand:
                return sandProperties;
            case Material::Water:
                return waterProperties;
            default:
                return emptyProperties; // Default to empty properties for unhandled materials
        }
    }
}