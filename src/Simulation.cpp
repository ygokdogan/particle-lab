#include <vector>

#include "sandbox_lab/MaterialProperties.hpp"
#include "sandbox_lab/Simulation.hpp"
#include "sandbox_lab/GasPhysics.hpp"
#include "sandbox_lab/LiquidPhysics.hpp"
#include "sandbox_lab/PowderPhysics.hpp"
#include "sandbox_lab/ReactionSystem.hpp"

namespace sandbox {
    void Simulation::update(Grid &grid) {
        currentFrame++;
        GasPhysics gasPhysics(rng, randomDirection, currentFrame);
        ReactionSystem reactions(rng, gasPhysics);
        LiquidPhysics liquidPhysics(rng, randomDirection);
        PowderPhysics powderPhysics(rng, randomDirection, currentFrame, liquidPhysics);

        int startX = scanLeftToRight ? 0 : grid.getWidth() - 1;
        int endX = scanLeftToRight ? grid.getWidth() : -1;
        int stepX = scanLeftToRight ? 1 : -1;

        for (int y = grid.getHeight() - 1; y >= 0; --y) {
            for (int x = startX; x != endX; x += stepX) {
                Cell &cell = grid.getCell(x, y);

                if (cell.gas != Material::Empty && cell.lastGasUpdatedFrame != currentFrame) {
                    gasPhysics.updateGas(grid, x, y);
                }

                if (grid.getLiquidAmount(x, y, Material::Lava) > 0.0f) {
                    reactions.updateLava(grid, x, y);
                }

                std::vector<Material> materials;

                for (const LiquidLayer &layer : cell.liquids) {
                    materials.push_back(layer.material);
                }
                for (Material material : materials) {
                    if (grid.getLiquidAmount(x, y, material) > 0.0f) {
                        liquidPhysics.updateLiquid(grid, x, y, material);
                    }
                }

                if (cell.lastUpdatedFrame == currentFrame) {
                    continue;
                }

                const MaterialProperties &properties = getMaterialProperties(cell.solid);

                switch (properties.behaviour) {
                case Behaviour::Powder:
                    powderPhysics.updatePowder(grid, x, y);
                    break;
                case Behaviour::Static:
                    if (cell.solid == Material::Wood) {
                        reactions.updateWood(grid, x, y);
                    }
                    break;
                default:
                    break;
                }
            }
        }

        reactions.coolLava(grid);
        scanLeftToRight = !scanLeftToRight;
    }

} // namespace sandbox