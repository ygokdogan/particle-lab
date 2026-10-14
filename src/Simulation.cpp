#include <utility>
#include "sandbox_lab/Simulation.hpp"
#include "sandbox_lab/MaterialProperties.hpp"

void sandbox::Simulation::update(Grid &grid) {
    for (int y = grid.getHeight() - 1; y >= 0; --y) {
        for (int x = 0; x < grid.getWidth(); ++x) {
            Cell& cell = grid.getCell(x, y);
            const MaterialProperties& properties = getMaterialProperties(cell.material);

            switch (properties.behaviour) {
                case Behaviour::Powder:
                    updatePowder(grid, x, y);
                    break;
                default:
                    break;
            }
        }
    }
}

void sandbox::Simulation::updatePowder(Grid& grid, int x, int y) {
    if (!grid.isEmptyCell(x, y+1)) {
        return;
    }

    std::swap(grid.getCell(x, y), grid.getCell(x, y+1));
}