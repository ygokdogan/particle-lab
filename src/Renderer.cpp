#include <raylib.h>

#include "sandbox_lab/MaterialProperties.hpp"
#include "sandbox_lab/Renderer.hpp"


namespace sandbox {

    Renderer::Renderer(int cellSize) : cellSize(cellSize) {
    }
    
    void Renderer::render(const Grid& grid) {
        for (int y = 0; y < grid.getHeight(); ++y) {
            for (int x = 0; x < grid.getWidth(); ++x) {
                const Cell& currentCell = grid.getCell(x,y);

                if (currentCell.solid != Material::Empty) {
                    renderSolidCell(currentCell, x, y);
                }
                else if (!currentCell.liquids.empty()) {
                    renderLiquidCell(currentCell, x, y);
                }
            }
        }
    }

    void Renderer::renderSolidCell(const Cell& cell, int x, int y) const {
        const MaterialProperties& props = getMaterialProperties(cell.solid);

        ::Color color {props.color.r, props.color.g, props.color.b, 255};
        DrawRectangle(x * cellSize, y * cellSize, cellSize, cellSize, color);
    }

    void Renderer::renderLiquidCell(const Cell& cell, int x, int y) const {
        constexpr float renderThreshold = 0.25f;

        const LiquidLayer* dominantLayer = nullptr;
        for (const LiquidLayer& layer : cell.liquids) {
            if (dominantLayer == nullptr || layer.amount > dominantLayer->amount) {
                dominantLayer = &layer;
            }
        }

        if(dominantLayer == nullptr || dominantLayer->amount < renderThreshold) {
            return;
        }

        const MaterialProperties& props = getMaterialProperties(dominantLayer->material);
        ::Color color{props.color.r, props.color.g, props.color.b, 255};
        DrawRectangle(x * cellSize, y * cellSize, cellSize, cellSize, color);
    }
}