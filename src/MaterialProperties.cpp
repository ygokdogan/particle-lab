#include "sandbox_lab/MaterialProperties.hpp"

namespace sandbox {
    const MaterialProperties &getMaterialProperties(Material material) {
        static const MaterialProperties emptyProperties{Behaviour::None, {0, 0, 0}, 0.0f};
        static const MaterialProperties stoneProperties{Behaviour::Static, {110, 115, 125}, 2500.0f};
        static const MaterialProperties sandProperties{Behaviour::Powder, {230, 200, 90}, 1600.0f};
        static const MaterialProperties waterProperties{Behaviour::Liquid, {50, 120, 230}, 1000.0f};
        static const MaterialProperties oilProperties{Behaviour::Liquid, {169, 139, 45}, 900.0f};

        static const MaterialProperties woodProperties{Behaviour::Static, {139, 90, 43}, 700.0f};
        static const MaterialProperties lavaProperties{Behaviour::Liquid, {244, 117, 40}, 2800.0f};
        static const MaterialProperties smokeProperties{Behaviour::Gas, {160, 165, 175}, 0.5f};
        static const MaterialProperties coolingRockProperties{Behaviour::Powder, {65, 68, 75}, 3000.0f,};
        static const MaterialProperties steamProperties{Behaviour::Gas, {210, 218, 225}, 0.3f};

        switch (material) {
        case Material::Empty:
            return emptyProperties;
        case Material::Stone:
            return stoneProperties;
        case Material::Sand:
            return sandProperties;
        case Material::Water:
            return waterProperties;
        case Material::Oil:
            return oilProperties;
        case Material::Wood:
            return woodProperties;
        case Material::Lava:
            return lavaProperties;
        case Material::Smoke:
            return smokeProperties;
        case Material::Steam:
            return steamProperties;
        case Material::CoolingRock:
            return coolingRockProperties;
        default:
            return emptyProperties; // Default to empty properties for unhandled materials
        }
    }
} // namespace sandbox